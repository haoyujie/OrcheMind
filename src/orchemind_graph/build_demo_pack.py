"""生成演示/测试包：第一层穷尽查询 + 第二层八关系大三元组，不改 V02 拓扑。"""



from __future__ import annotations



import json

import sys

from pathlib import Path



ROOT = Path(__file__).resolve().parents[2]

GRAPH = ROOT / "data" / "ontology_graph" / "v02_graph.json"

RELATIONS = ROOT / "data" / "rcc8" / "relations.json"

OPERATOR = ROOT / "data" / "rcc8" / "operator_prior.json"

ANGLES = ROOT / "data" / "rcc8" / "angles_f0.json"

OUT_TRIPLES = ROOT / "data" / "triples" / "rcc8_demo_large.jsonl"

OUT_SCENARIOS = ROOT / "data" / "demo" / "scenarios.json"

OUT_LAYER2 = ROOT / "data" / "demo" / "layer2_verb_catalog.json"





def load(path: Path) -> dict:

    return json.loads(path.read_text(encoding="utf-8"))





def add(

    rows: list[dict],

    head: str,

    relation: str,

    tail: str,

    *,

    scene: str,

    note: str = "",

) -> None:

    item = {

        "head": head,

        "relation": relation,

        "tail": tail,

        "split": "demo",

        "scene": scene,

        "note": note,

    }

    rows.append(item)





def build_triples() -> list[dict]:

    """抽象区域大场景：八个 RCC8 都出现，并显式写出逆对。"""

    rows: list[dict] = []



    # ---- 场景 A：同心嵌套（NTPP / NTPPi / TPP / TPPi / DC）----

    # core ⊂ mid ⊂ shell ⊂ world；rim 正切贴在 shell 内侧

    nest = [

        ("core", "NTPP", "mid"),

        ("mid", "NTPP", "shell"),

        ("shell", "NTPP", "world"),

        ("core", "NTPP", "shell"),

        ("core", "NTPP", "world"),

        ("mid", "NTPP", "world"),

        ("rim", "TPP", "shell"),

        ("rim", "NTPP", "world"),

        ("core", "DC", "rim"),

        ("mid", "DC", "rim"),

        # 显式逆

        ("mid", "NTPPi", "core"),

        ("shell", "NTPPi", "mid"),

        ("shell", "NTPPi", "core"),

        ("world", "NTPPi", "shell"),

        ("world", "NTPPi", "mid"),

        ("world", "NTPPi", "core"),

        ("world", "NTPPi", "rim"),

        ("shell", "TPPi", "rim"),

    ]

    for h, r, t in nest:

        add(rows, h, r, t, scene="nest", note="同心嵌套与逆对")



    # ---- 场景 B：左右相离与外切、相交（DC / EC / PO）----

    side = [

        ("west", "DC", "east"),

        ("west", "EC", "gap_w"),

        ("east", "EC", "gap_e"),

        ("gap_w", "DC", "east"),

        ("gap_e", "DC", "west"),

        ("gap_w", "DC", "gap_e"),

        ("blend", "PO", "west"),

        ("blend", "PO", "east"),

        ("blend", "DC", "core"),

        ("blend", "NTPP", "world"),

        ("west", "NTPP", "world"),

        ("east", "NTPP", "world"),

        ("gap_w", "NTPP", "world"),

        ("gap_e", "NTPP", "world"),

        ("west", "DC", "core"),

        ("east", "DC", "core"),

        ("west", "DC", "shell"),

        ("east", "DC", "shell"),

        ("gap_w", "DC", "shell"),

        ("gap_e", "DC", "shell"),

    ]

    for h, r, t in side:

        add(rows, h, r, t, scene="side", note="左右相离/外切/部分相交")



    # ---- 场景 C：等同与副本（EQ）----

    eq = [

        ("badge", "EQ", "badge_copy"),

        ("badge", "NTPP", "world"),

        ("badge_copy", "NTPP", "world"),

        ("badge", "DC", "west"),

        ("badge_copy", "DC", "east"),

        ("badge", "DC", "core"),

        ("badge_copy", "DC", "core"),

        ("mark", "EQ", "mark_same"),

        ("mark", "TPP", "shell"),

        ("mark_same", "TPP", "shell"),

        ("mark", "DC", "core"),

        ("mark_same", "DC", "core"),

    ]

    for h, r, t in eq:

        add(rows, h, r, t, scene="equal", note="等同副本")



    # ---- 场景 D：南北条带，补齐 EC/DC/PO 密度 ----

    strip = [

        ("north", "DC", "south"),

        ("north", "EC", "belt"),

        ("south", "EC", "belt"),

        ("north", "DC", "west"),

        ("south", "DC", "east"),

        ("belt", "PO", "blend"),

        ("pocket_n", "NTPP", "north"),

        ("pocket_s", "NTPP", "south"),

        ("pocket_n", "DC", "south"),

        ("pocket_s", "DC", "north"),

        ("pocket_n", "DC", "pocket_s"),

        ("north", "NTPP", "world"),

        ("south", "NTPP", "world"),

        ("belt", "NTPP", "world"),

        ("pocket_n", "NTPP", "world"),

        ("pocket_s", "NTPP", "world"),

        ("north", "DC", "core"),

        ("south", "DC", "core"),

        ("belt", "DC", "core"),

        ("pocket_n", "DC", "rim"),

        ("pocket_s", "DC", "rim"),

    ]

    for h, r, t in strip:

        add(rows, h, r, t, scene="strip", note="南北条带")



    # ---- 场景 E：更多 TPP/TPPi / NTPP/NTPPi 对，便于第二层穷举 ----

    more = [

        ("inset_a", "NTPP", "frame"),

        ("inset_b", "NTPP", "frame"),

        ("lip", "TPP", "frame"),

        ("inset_a", "DC", "lip"),

        ("inset_b", "DC", "lip"),

        ("inset_a", "DC", "inset_b"),

        ("frame", "NTPPi", "inset_a"),

        ("frame", "NTPPi", "inset_b"),

        ("frame", "TPPi", "lip"),

        ("frame", "NTPP", "world"),

        ("inset_a", "NTPP", "world"),

        ("inset_b", "NTPP", "world"),

        ("lip", "NTPP", "world"),

        ("frame", "DC", "west"),

        ("frame", "DC", "east"),

        ("frame", "DC", "core"),

        ("dock", "EC", "frame"),

        ("dock", "DC", "inset_a"),

        ("dock", "DC", "core"),

        ("dock", "NTPP", "world"),

        ("halo", "PO", "frame"),

        ("halo", "PO", "dock"),

        ("halo", "DC", "core"),

        ("halo", "NTPP", "world"),

        ("ring_eq", "EQ", "ring_twin"),

        ("ring_eq", "TPP", "world"),

        ("ring_twin", "TPP", "world"),

        ("ring_eq", "DC", "core"),

        ("ring_twin", "DC", "west"),

    ]

    for h, r, t in more:

        add(rows, h, r, t, scene="frame", note="画框与停靠")



    # ---- scene F: denser EC/PO/EQ/TPPi ----
    grid = [
        ("tile_11", "EC", "tile_12"),
        ("tile_12", "EC", "tile_13"),
        ("tile_11", "EC", "tile_21"),
        ("tile_12", "EC", "tile_22"),
        ("tile_13", "EC", "tile_23"),
        ("tile_21", "EC", "tile_22"),
        ("tile_22", "EC", "tile_23"),
        ("tile_11", "DC", "tile_13"),
        ("tile_11", "DC", "tile_23"),
        ("tile_13", "DC", "tile_21"),
        ("tile_21", "DC", "tile_23"),
        ("overlap_row", "PO", "tile_11"),
        ("overlap_row", "PO", "tile_12"),
        ("overlap_col", "PO", "tile_12"),
        ("overlap_col", "PO", "tile_22"),
        ("tile_11", "NTPP", "world"),
        ("tile_12", "NTPP", "world"),
        ("tile_13", "NTPP", "world"),
        ("tile_21", "NTPP", "world"),
        ("tile_22", "NTPP", "world"),
        ("tile_23", "NTPP", "world"),
        ("overlap_row", "NTPP", "world"),
        ("overlap_col", "NTPP", "world"),
        ("tile_11", "DC", "core"),
        ("tile_22", "DC", "core"),
        ("cap_a", "EQ", "cap_b"),
        ("cap_a", "EQ", "cap_c"),
        ("cap_b", "EQ", "cap_c"),
        ("cap_a", "TPP", "frame"),
        ("cap_b", "TPP", "frame"),
        ("cap_c", "TPP", "frame"),
        ("frame", "TPPi", "cap_a"),
        ("frame", "TPPi", "cap_b"),
        ("frame", "TPPi", "cap_c"),
        ("shell", "TPPi", "mark"),
        ("shell", "TPPi", "mark_same"),
        ("world", "TPPi", "ring_eq"),
        ("world", "TPPi", "ring_twin"),
        ("cap_a", "DC", "core"),
        ("cap_b", "DC", "west"),
        ("cap_c", "DC", "east"),
        ("tile_11", "DC", "west"),
        ("tile_23", "DC", "east"),
        ("overlap_row", "DC", "core"),
        ("overlap_col", "DC", "shell"),
    ]
    for h, r, t in grid:
        add(rows, h, r, t, scene="grid", note="grid densify")

    # 去重保序

    seen: set[tuple[str, str, str]] = set()

    unique: list[dict] = []

    for row in rows:

        key = (row["head"], row["relation"], row["tail"])

        if key in seen:

            continue

        seen.add(key)

        unique.append(row)

    return unique





def build_scenarios(graph: dict, triples: list[dict]) -> dict:

    nodes = [node["name"] for node in graph["nodes"]]

    layer1 = []

    for name in nodes:

        # 问题里带上节点名，便于视图命中

        if name.startswith(("DC", "EC", "PO", "EQ", "TPP", "NTPP")):

            query = f"协同性里的{name}"

            group = "rcc8"

        elif name in ("量", "质", "关系", "模态"):

            query = f"康德范畴{name}"

            group = "root"

        else:

            query = f"关于{name}"

            group = "category"

        layer1.append(

            {

                "label": name.split(" ")[0] if " " in name else name,

                "query": query,

                "expect_node": name,

                "group": group,

            }

        )



    by_rel: dict[str, list[dict]] = {}

    for row in triples:

        by_rel.setdefault(row["relation"], []).append(

            {"head": row["head"], "tail": row["tail"], "scene": row["scene"]}

        )



    layer2 = []

    for code in ("DC", "EC", "PO", "EQ", "TPP", "TPPi", "NTPP", "NTPPi"):

        examples = by_rel.get(code, [])[:5]

        layer2.append(

            {

                "code": code,

                "label": code,

                "query": code if code in ("TPPi", "NTPPi") else f"{code}",

                "n_triples": len(by_rel.get(code, [])),

                "examples": examples,

            }

        )



    return {

        "note": "第一层穷尽本体地图节点；第二层穷尽 RCC8 八个动词。供视图按钮与人工验收，不改 V02。",

        "layer1_queries": layer1,

        "layer2_verbs": layer2,

        "preset_buttons": [

            {"label": "量·单一性", "query": "关于单一性"},

            {"label": "质·实在性", "query": "关于实在性"},

            {"label": "关系·协同性", "query": "关于协同性（共存）"},

            {"label": "模态·必然性", "query": "关于必然性与偶然性"},

            {"label": "DC 相离", "query": "DC 相离"},

            {"label": "EC 外切", "query": "EC 外切"},

            {"label": "PO 相交", "query": "PO 部分相交"},

            {"label": "EQ 等同", "query": "EQ 等同"},

            {"label": "TPP", "query": "TPP 正切真子集"},

            {"label": "TPPi", "query": "TPPi"},

            {"label": "NTPP", "query": "NTPP 非切真子集"},

            {"label": "NTPPi", "query": "NTPPi"},

            {"label": "穿过(未入图)", "query": "通孔穿过焊盘"},

        ],

    }





def build_layer2_catalog(triples: list[dict]) -> dict:

    relations = load(RELATIONS)["relations"]

    operator = {item["code"]: item["deg"] for item in load(OPERATOR)["verbs"]}

    display = {item["code"]: item["deg"] for item in load(ANGLES)["verbs"]}

    by_rel: dict[str, list[dict]] = {}

    for row in triples:

        by_rel.setdefault(row["relation"], []).append(row)



    verbs = []

    for item in relations:

        code = item["code"]

        examples = by_rel.get(code, [])

        verbs.append(

            {

                "code": code,

                "name_zh": item["name_zh"],

                "name_en": item["name_en"],

                "inverse": item["inverse"],

                "display_deg": display[code],

                "operator_prior_deg": operator[code],

                "n_demo_triples": len(examples),

                "examples": [

                    {"head": row["head"], "tail": row["tail"], "scene": row["scene"]}

                    for row in examples[:8]

                ],

            }

        )

    return {

        "note": "第二层关系圆上八个动词的穷举目录：语义扇区角、算子先验角、演示三元组。",

        "n_verbs": len(verbs),

        "n_demo_triples_total": len(triples),

        "verbs": verbs,

    }





def main() -> None:

    graph = load(GRAPH)

    triples = build_triples()

    scenarios = build_scenarios(graph, triples)

    catalog = build_layer2_catalog(triples)



    OUT_TRIPLES.parent.mkdir(parents=True, exist_ok=True)

    OUT_SCENARIOS.parent.mkdir(parents=True, exist_ok=True)

    OUT_TRIPLES.write_text(

        "\n".join(json.dumps(row, ensure_ascii=False) for row in triples) + "\n",

        encoding="utf-8",

    )

    OUT_SCENARIOS.write_text(

        json.dumps(scenarios, ensure_ascii=False, indent=2), encoding="utf-8"

    )

    OUT_LAYER2.write_text(

        json.dumps(catalog, ensure_ascii=False, indent=2), encoding="utf-8"

    )



    counts: dict[str, int] = {}

    for row in triples:

        counts[row["relation"]] = counts.get(row["relation"], 0) + 1

    ents = {row["head"] for row in triples} | {row["tail"] for row in triples}

    print(f"triples={len(triples)} entities={len(ents)}")

    print("by_relation", dict(sorted(counts.items())))

    print(f"wrote {OUT_TRIPLES}")

    print(f"wrote {OUT_SCENARIOS}")

    print(f"wrote {OUT_LAYER2}")





if __name__ == "__main__":

    main()


