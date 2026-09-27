"""构建 V01 裸图：康德四大类、十二范畴，以及 RCC8。"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_graph.graph_model import Node, RawOntologyGraph

OUT_FILE = ROOT / "data" / "ontology_graph" / "v01_graph.json"


def _hang(graph: RawOntologyGraph, parent: Node, names: list[str]) -> list[Node]:
    children: list[Node] = []
    for name in names:
        child = graph.add_node(name)
        graph.add_link(parent.id, child.id)
        children.append(child)
    return children


def build_v01_raw_graph() -> RawOntologyGraph:
    graph = RawOntologyGraph()

    quantity = graph.add_node("量")
    quality = graph.add_node("质")
    relation = graph.add_node("关系")
    modality = graph.add_node("模态")

    _hang(graph, quantity, ["单一性", "多数性", "全体性"])
    _hang(graph, quality, ["实在性", "否定性", "限制性"])
    relation_children = _hang(
        graph,
        relation,
        ["实体与偶性", "因果性", "协同性（共存）"],
    )
    _hang(
        graph,
        modality,
        ["可能性与不可能性", "存在性与不存在性", "必然性与偶然性"],
    )

    community = relation_children[2]
    _hang(
        graph,
        community,
        [
            "DC 相离",
            "EC 外切",
            "PO 部分相交",
            "EQ 等同",
            "TPP 正切真子集",
            "TPPi",
            "NTPP 非切真子集",
            "NTPPi",
        ],
    )
    return graph


def main() -> None:
    graph = build_v01_raw_graph()
    graph.save_json(OUT_FILE)
    print(f"OrcheMind V01 裸图已保存：{OUT_FILE}")
    print(f"节点总数：{len(graph.nodes)}")
    print(f"连线总数：{len(graph.links)}")


if __name__ == "__main__":
    main()
