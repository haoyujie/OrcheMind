"""校验 V02：V01 的节点和 20 条边保持原样，类型、方向、端口和逆对齐全。"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_graph.graph_model import RawOntologyGraph
from orchemind_graph.typed_model import TypedOntologyGraph

V01_FILE = ROOT / "data" / "ontology_graph" / "v01_graph.json"
V02_FILE = ROOT / "data" / "ontology_graph" / "v02_graph.json"
SNAPSHOT_V01 = ROOT / "snapshot" / "v01_raw_graph" / "v01_graph.json"


def validate() -> list[str]:
    errors: list[str] = []
    if V01_FILE.read_bytes() != SNAPSHOT_V01.read_bytes():
        errors.append("V01 工作文件与快照不一致，已停止")
        return errors

    raw = RawOntologyGraph.load_json(V01_FILE)
    typed = TypedOntologyGraph.load_json(V02_FILE)
    raw_nodes = {node.id: node.name for node in raw.nodes}
    typed_nodes = {node.id: node.name for node in typed.nodes}
    if raw_nodes != typed_nodes:
        errors.append("V02 改动了 V01 的节点身份或名称")

    raw_edges = {(link.id, link.source_id, link.target_id) for link in raw.links}
    typed_by_id = {link.id: link for link in typed.links}
    if not raw_edges <= {
        (link.id, link.source_id, link.target_id) for link in typed.links
    }:
        errors.append("V01 的连线没有全部保留")

    category = rcc8 = inverse = 0
    for link_id, source_id, target_id in raw_edges:
        link = typed_by_id[link_id]
        if not link.directed:
            errors.append(f"原有连线被改成无方向：{link_id}")
        if link.link_type == "has_category":
            category += 1
        elif link.link_type == "has_rcc8":
            rcc8 += 1
        else:
            errors.append(f"原有连线类型不对：{raw_nodes[source_id]} -> {link.link_type}")
        _require_port(typed, source_id, link.link_type, "out", errors)
        _require_port(typed, target_id, link.link_type, "in", errors)

    inverse_pairs = {
        frozenset({raw_nodes[link.source_id], raw_nodes[link.target_id]})
        for link in typed.links
        if link.link_type == "inverse_of"
    }
    expected_pairs = {
        frozenset({"TPP 正切真子集", "TPPi"}),
        frozenset({"NTPP 非切真子集", "NTPPi"}),
    }
    if inverse_pairs != expected_pairs:
        errors.append(f"逆对不对：{sorted(str(pair) for pair in inverse_pairs)}")
    for link in typed.links:
        if link.link_type != "inverse_of":
            continue
        inverse += 1
        if link.directed:
            errors.append("inverse_of 应是无方向连线")
        if link.id in {item.id for item in raw.links}:
            errors.append("逆对占用了 V01 连线 id")
        _require_port(typed, link.source_id, "inverse_of", "undirected", errors)
        _require_port(typed, link.target_id, "inverse_of", "undirected", errors)

    if category != 12 or rcc8 != 8 or inverse != 2 or len(typed.links) != 22:
        errors.append(
            f"规模不符：范畴 {category}，RCC8 {rcc8}，逆对 {inverse}，总连线 {len(typed.links)}"
        )
    return errors


def _require_port(graph, node_id: str, link_type: str, role: str, errors: list[str]) -> None:
    node = next(item for item in graph.nodes if item.id == node_id)
    if not any(port.link_type == link_type and port.role == role for port in node.ports):
        errors.append(f"{node.name} 缺少端口 {link_type}/{role}")


def main() -> None:
    errors = validate()
    if errors:
        for error in errors:
            print(error)
        raise SystemExit(1)
    print(f"V02 校验通过：{V02_FILE}")


if __name__ == "__main__":
    main()
