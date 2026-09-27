"""校验 V01 裸图：名称不重复、边不重复，且从四大类能走到全部节点。"""

from __future__ import annotations

import sys
from pathlib import Path

import networkx as nx

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_graph.graph_model import RawOntologyGraph

GRAPH_FILE = ROOT / "data" / "ontology_graph" / "v01_graph.json"

TOP_CATEGORIES = ("量", "质", "关系", "模态")
EXPECTED_NAMES = (
    *TOP_CATEGORIES,
    "单一性",
    "多数性",
    "全体性",
    "实在性",
    "否定性",
    "限制性",
    "实体与偶性",
    "因果性",
    "协同性（共存）",
    "可能性与不可能性",
    "存在性与不存在性",
    "必然性与偶然性",
    "DC 相离",
    "EC 外切",
    "PO 部分相交",
    "EQ 等同",
    "TPP 正切真子集",
    "TPPi",
    "NTPP 非切真子集",
    "NTPPi",
)


def validate(graph: RawOntologyGraph) -> list[str]:
    errors: list[str] = []
    names = [node.name for node in graph.nodes]
    if len(names) != len(set(names)):
        seen: set[str] = set()
        duplicates = sorted({name for name in names if name in seen or seen.add(name)})
        errors.append(f"节点名重复：{duplicates}")

    missing = [name for name in EXPECTED_NAMES if name not in names]
    extra = [name for name in names if name not in EXPECTED_NAMES]
    if missing:
        errors.append(f"缺少节点：{missing}")
    if extra:
        errors.append(f"多余节点：{extra}")

    edges = [(link.source_id, link.target_id) for link in graph.links]
    if len(edges) != len(set(edges)):
        errors.append("存在重复连线")

    id_to_name = {node.id: node.name for node in graph.nodes}
    unknown = [
        edge
        for edge in edges
        if edge[0] not in id_to_name or edge[1] not in id_to_name
    ]
    if unknown:
        errors.append(f"连线指向不存在的节点：{len(unknown)}")

    undirected = nx.Graph()
    undirected.add_nodes_from(id_to_name)
    undirected.add_edges_from(edges)
    roots = [node.id for node in graph.nodes if node.name in TOP_CATEGORIES]
    reachable: set[str] = set()
    for root in roots:
        reachable |= nx.node_connected_component(undirected, root)
    unreachable = [
        id_to_name[node_id] for node_id in id_to_name if node_id not in reachable
    ]
    if unreachable:
        errors.append(f"从四大类走不到：{sorted(unreachable)}")

    if len(graph.nodes) != 24 or len(graph.links) != 20:
        errors.append(
            f"规模不符：节点 {len(graph.nodes)}（应为 24），连线 {len(graph.links)}（应为 20）"
        )
    return errors


def main() -> None:
    graph = RawOntologyGraph.load_json(GRAPH_FILE)
    errors = validate(graph)
    if errors:
        for error in errors:
            print(error)
        raise SystemExit(1)
    print(f"V01 校验通过：{GRAPH_FILE}")
    print(f"节点 {len(graph.nodes)}，连线 {len(graph.links)}")


if __name__ == "__main__":
    main()
