"""导出与视图同构图的静态 PNG，供 README 与各 snapshot 版本页引用。"""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib import font_manager
from matplotlib.patches import Circle

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

GRAPH_V02 = ROOT / "data" / "ontology_graph" / "v02_graph.json"
GRAPH_V01 = ROOT / "data" / "ontology_graph" / "v01_graph.json"
ANGLES_F0 = ROOT / "data" / "rcc8" / "angles_f0.json"
EMBED_V03 = ROOT / "data" / "ontology_graph" / "v03_embedding.json"
HERO = ROOT / "docs" / "figures" / "hero"
SNAP = ROOT / "snapshot"

_FONT_CANDIDATES = [
    Path(r"C:\Windows\Fonts\simhei.ttf"),
    Path(r"C:\Windows\Fonts\msyh.ttc"),
    Path(r"C:\Windows\Fonts\msyh.ttf"),
]
FONT = None
for _path in _FONT_CANDIDATES:
    if _path.exists():
        FONT = font_manager.FontProperties(fname=str(_path))
        break


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def layout(graph: dict) -> dict[str, tuple[float, float]]:
    children: dict[str, list[str]] = {node["id"]: [] for node in graph["nodes"]}
    incoming: set[str] = set()
    for link in graph["links"]:
        if not link.get("directed", True):
            continue
        children[link["source_id"]].append(link["target_id"])
        incoming.add(link["target_id"])
    pos: dict[str, tuple[float, float]] = {}
    roots = [node for node in graph["nodes"] if node["id"] not in incoming]
    column_width = 2.4
    for index, root in enumerate(roots):
        x = index * column_width
        pos[root["id"]] = (x, 0.0)
        y = -0.9
        for child_id in children[root["id"]]:
            pos[child_id] = (x, y)
            y -= 0.7
            for grand_id in children[child_id]:
                pos[grand_id] = (x, y)
                y -= 0.55
    return pos


def draw_map(graph: dict, out: Path, title: str) -> None:
    pos = layout(graph)
    names = {node["id"]: node["name"] for node in graph["nodes"]}
    fig, ax = plt.subplots(figsize=(11, 7), dpi=160)
    fig.patch.set_facecolor("#f4f1ea")
    ax.set_facecolor("#fffdf8")
    for link in graph["links"]:
        a = pos[link["source_id"]]
        b = pos[link["target_id"]]
        ax.plot([a[0], b[0]], [a[1], b[1]], color="#8b97a3", lw=1.2, zorder=1)
    for node_id, (x, y) in pos.items():
        ax.scatter([x], [y], s=36, c="#1c2430", zorder=2)
        ax.text(
            x,
            y + 0.18,
            names[node_id],
            ha="center",
            va="bottom",
            fontsize=8,
            color="#1c2430",
            fontproperties=FONT,
        )
    ax.set_title(title, fontsize=13, pad=12, fontproperties=FONT)
    ax.set_aspect("equal")
    ax.axis("off")
    fig.tight_layout()
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, bbox_inches="tight", facecolor=fig.get_facecolor())
    plt.close(fig)


def draw_circle(angles: dict, out: Path, title: str) -> None:
    fig, ax = plt.subplots(figsize=(6.2, 6.5), dpi=160)
    fig.patch.set_facecolor("#f4f1ea")
    ax.set_facecolor("#fffdf8")
    radius = 1.0
    ax.add_patch(Circle((0, 0), radius, fill=False, ec="#8b97a3", lw=1.4))
    ax.axhline(0, color="#e4ddd2", lw=1)
    ax.axvline(0, color="#e4ddd2", lw=1)
    for verb in angles["verbs"]:
        if verb.get("prior_deg") is not None:
            pr = math.radians(float(verb["prior_deg"]))
            ax.scatter(
                [math.cos(pr)],
                [math.sin(pr)],
                s=28,
                facecolors="none",
                edgecolors="#8b97a3",
                linewidths=1.0,
                zorder=2,
            )
        deg = float(verb.get("learned_deg", verb.get("deg", 0.0)))
        rad = math.radians(deg)
        x, y = math.cos(rad), math.sin(rad)
        ax.scatter([x], [y], s=42, c="#1c2430", zorder=3)
        delta = verb.get("delta_deg")
        label = f"{verb['code']} {deg:.1f}°"
        if delta is not None:
            label += f" Δ{float(delta):.1f}"
        ax.text(
            1.22 * x,
            1.22 * y,
            label,
            ha="center",
            va="center",
            fontsize=8,
            color="#1c2430",
            fontproperties=FONT,
        )
    ax.set_xlim(-1.55, 1.55)
    ax.set_ylim(-1.55, 1.55)
    ax.set_aspect("equal")
    ax.axis("off")
    ax.set_title(title, fontsize=12, pad=10, fontproperties=FONT)
    fig.tight_layout()
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, bbox_inches="tight", facecolor=fig.get_facecolor())
    plt.close(fig)


def main() -> None:
    v01 = load_json(GRAPH_V01)
    v02 = load_json(GRAPH_V02)
    f0 = load_json(ANGLES_F0)
    draw_map(v02, HERO / "ontology_map.png", "OrcheMind · 本体地图（V02）")
    if EMBED_V03.exists():
        v03 = load_json(EMBED_V03)
        draw_circle(v03, HERO / "relation_circle.png", "OrcheMind · 第一谐波关系圆（V03）")
        draw_circle(
            v03,
            SNAP / "v03_embedding_base" / "figures" / "relation_circle.png",
            "V03 · 算子角（实心）与语义先验（空心）",
        )
    else:
        draw_circle(f0, HERO / "relation_circle.png", "OrcheMind · 第一谐波关系圆（先验）")

    draw_map(v01, SNAP / "v01_raw_graph" / "figures" / "ontology_map.png", "V01 · 裸拓扑地图")
    draw_map(v02, SNAP / "v02_typed_link" / "figures" / "ontology_map.png", "V02 · 有类型连线地图")
    draw_circle(
        f0,
        SNAP / "v02_typed_link" / "figures" / "relation_circle_prior.png",
        "V02 阶段 · 人工语义扇区（angles_f0）",
    )
    print(f"已导出首页图到 {HERO}")
    print("已导出各 snapshot/*/figures/")


if __name__ == "__main__":
    main()
