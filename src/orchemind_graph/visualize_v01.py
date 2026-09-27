"""可选：用 matplotlib 查看 V01 裸图。通过标准是 validate_v01.py。"""

from __future__ import annotations

import sys
from pathlib import Path

import matplotlib.pyplot as plt
import networkx as nx

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_graph.graph_model import RawOntologyGraph

GRAPH_FILE = ROOT / "data" / "ontology_graph" / "v01_graph.json"


def render_graph(json_path: Path) -> None:
    graph = RawOntologyGraph.load_json(json_path)
    drawn = nx.Graph()
    labels: dict[str, str] = {}
    for node in graph.nodes:
        drawn.add_node(node.id)
        labels[node.id] = node.name
    for link in graph.links:
        drawn.add_edge(link.source_id, link.target_id)

    pos = nx.spring_layout(drawn, seed=42)
    plt.figure(figsize=(12, 8))
    nx.draw(drawn, pos, labels=labels, node_size=1300, font_size=8)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    render_graph(GRAPH_FILE)
