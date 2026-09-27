"""由 V01 快照增量生成 V02：原有 20 条边只加类型和方向，另加两条逆对。"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_graph.graph_model import RawOntologyGraph
from orchemind_graph.typed_model import (
    Port,
    TypedLink,
    TypedNode,
    TypedOntologyGraph,
    stable_id,
)

V01_FILE = ROOT / "data" / "ontology_graph" / "v01_graph.json"
OUT_FILE = ROOT / "data" / "ontology_graph" / "v02_graph.json"

TOP_CATEGORIES = {"量", "质", "关系", "模态"}
COMMUNITY = "协同性（共存）"
INVERSE_PAIRS = (
    ("TPP 正切真子集", "TPPi"),
    ("NTPP 非切真子集", "NTPPi"),
)


def _port(node_id: str, link_type: str, role: str) -> Port:
    return Port(
        id=stable_id(f"port:{node_id}:{link_type}:{role}"),
        link_type=link_type,
        role=role,
    )


def build_v02(raw: RawOntologyGraph) -> TypedOntologyGraph:
    name_of = {node.id: node.name for node in raw.nodes}
    id_of = {node.name: node.id for node in raw.nodes}
    typed_links: list[TypedLink] = []
    roles: dict[str, set[tuple[str, str]]] = {node.id: set() for node in raw.nodes}

    for link in raw.links:
        source_name = name_of[link.source_id]
        if source_name in TOP_CATEGORIES:
            link_type = "has_category"
        elif source_name == COMMUNITY:
            link_type = "has_rcc8"
        else:
            raise ValueError(f"无法给连线分类：{source_name}")
        typed_links.append(
            TypedLink(
                id=link.id,
                source_id=link.source_id,
                target_id=link.target_id,
                link_type=link_type,
                directed=True,
            )
        )
        roles[link.source_id].add((link_type, "out"))
        roles[link.target_id].add((link_type, "in"))

    for left_name, right_name in INVERSE_PAIRS:
        left_id = id_of[left_name]
        right_id = id_of[right_name]
        typed_links.append(
            TypedLink(
                id=stable_id(f"inverse:{left_name}:{right_name}"),
                source_id=left_id,
                target_id=right_id,
                link_type="inverse_of",
                directed=False,
            )
        )
        roles[left_id].add(("inverse_of", "undirected"))
        roles[right_id].add(("inverse_of", "undirected"))

    nodes = [
        TypedNode(
            id=node.id,
            name=node.name,
            ports=[
                _port(node.id, link_type, role)
                for link_type, role in sorted(roles[node.id])
            ],
        )
        for node in raw.nodes
    ]
    return TypedOntologyGraph(nodes=nodes, links=typed_links)


def main() -> None:
    graph = build_v02(RawOntologyGraph.load_json(V01_FILE))
    graph.save_json(OUT_FILE)
    print(f"OrcheMind V02 已保存：{OUT_FILE}")
    print(f"节点总数：{len(graph.nodes)}")
    print(f"连线总数：{len(graph.links)}")


if __name__ == "__main__":
    main()
