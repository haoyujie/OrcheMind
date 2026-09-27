"""Read the RCC8 axiom pack and check the composition table plus abstract triples."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RCC8 = ROOT / "data" / "rcc8"
TRIPLES = ROOT / "data" / "triples" / "rcc8_abstract.jsonl"
TESTS = ROOT / "data" / "triples" / "rcc8_composition_tests.jsonl"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def load_jsonl(path: Path) -> list[dict]:
    rows = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line:
            rows.append(json.loads(line))
    return rows


def inverse_map(relations: dict) -> dict[str, str]:
    return {item["code"]: item["inverse"] for item in relations["relations"]}


def compose(table: dict, r1: str, r2: str) -> set[str]:
    return set(table["composition"][r1][r2])


def check_table(table: dict, inv: dict[str, str]) -> list[str]:
    errors: list[str] = []
    order = table["order"]
    if order != ["DC", "EC", "PO", "TPP", "NTPP", "TPPi", "NTPPi", "EQ"]:
        errors.append("关系顺序与 RCC8 八关系不一致")
    all_rel = set(order)
    comp = table["composition"]
    if set(comp) != all_rel:
        errors.append("组合表行不齐")
    for r1 in order:
        if set(comp[r1]) != all_rel:
            errors.append(f"组合表列不齐：{r1}")
            continue
        for r2 in order:
            cell = set(comp[r1][r2])
            if not cell:
                errors.append(f"空格：{r1} ; {r2}")
            if not cell <= all_rel:
                errors.append(f"未知关系：{r1} ; {r2} -> {sorted(cell - all_rel)}")
            if compose(table, "EQ", r1) != {r1} or compose(table, r1, "EQ") != {r1}:
                errors.append(f"EQ 不是单位元：{r1}")
            converse = {inv[item] for item in cell}
            flipped = compose(table, inv[r2], inv[r1])
            if converse != flipped:
                errors.append(
                    f"逆元定律失败：({r1};{r2})^-1 = {sorted(converse)}，"
                    f"{inv[r2]};{inv[r1]} = {sorted(flipped)}"
                )
    expected = {
        ("NTPPi", "NTPPi"): {"NTPPi"},
        ("NTPP", "NTPP"): {"NTPP"},
        ("EQ", "TPP"): {"TPP"},
        ("TPP", "EC"): {"DC", "EC"},
        ("EC", "TPP"): {"EC", "PO", "TPP", "NTPP"},
        ("DC", "EQ"): {"DC"},
        ("TPP", "NTPP"): {"NTPP"},
        ("DC", "DC"): all_rel,
        ("NTPP", "NTPPi"): all_rel,
        ("TPPi", "TPPi"): {"TPPi", "NTPPi"},
    }
    for (r1, r2), allowed in expected.items():
        got = compose(table, r1, r2)
        if got != allowed:
            errors.append(f"样例不符：{r1};{r2} 得到 {sorted(got)}，应为 {sorted(allowed)}")
    return errors


def check_tests(table: dict, tests: list[dict]) -> list[str]:
    errors: list[str] = []
    if len(tests) != 10:
        errors.append(f"组合题应为 10 条，实际 {len(tests)}")
    for index, row in enumerate(tests, start=1):
        got = compose(table, row["r1"], row["r2"])
        allowed = set(row["allowed"])
        if got != allowed:
            errors.append(
                f"第 {index} 题 {row['r1']};{row['r2']} 文件为 {sorted(allowed)}，表为 {sorted(got)}"
            )
    return errors


def check_triples(table: dict, inv: dict[str, str], rows: list[dict]) -> list[str]:
    errors: list[str] = []
    codes = set(inv)
    if not 30 <= len(rows) <= 50:
        errors.append(f"抽象三元组应为 30 到 50 条，实际 {len(rows)}")
    facts: dict[tuple[str, str], str] = {}
    for row in rows:
        if row.get("split") != "train":
            errors.append(f"抽象三元组 split 不是 train：{row}")
        if row["relation"] not in codes:
            errors.append(f"关系不在 RCC8 中：{row}")
        key = (row["head"], row["tail"])
        if key in facts and facts[key] != row["relation"]:
            errors.append(f"同一对区域有两个关系：{key}")
        facts[key] = row["relation"]
        back = (row["tail"], row["head"])
        if back in facts and facts[back] != inv[row["relation"]]:
            errors.append(
                f"逆关系不一致：{row['head']} {row['relation']} {row['tail']}，"
                f"反向却是 {facts[back]}"
            )
    by_head: dict[str, list[tuple[str, str]]] = {}
    for (head, tail), relation in facts.items():
        by_head.setdefault(head, []).append((relation, tail))
    for (a, b), r1 in facts.items():
        for r2, c in by_head.get(b, []):
            if (a, c) not in facts:
                continue
            r3 = facts[(a, c)]
            if r3 not in compose(table, r1, r2):
                errors.append(f"与组合表冲突：{a} {r1} {b}，{b} {r2} {c}，但 {a} {r3} {c}")
    return errors


def main() -> None:
    relations = load_json(RCC8 / "relations.json")
    table = load_json(RCC8 / "composition.json")
    inv = inverse_map(relations)
    errors = []
    errors.extend(check_table(table, inv))
    errors.extend(check_tests(table, load_jsonl(TESTS)))
    errors.extend(check_triples(table, inv, load_jsonl(TRIPLES)))
    if errors:
        for error in errors:
            print(error)
        raise SystemExit(1)
    print("RCC8 组合表验收通过")
    print(f"抽象三元组：{len(load_jsonl(TRIPLES))} 条")
    print(f"组合题：{len(load_jsonl(TESTS))} 条")


if __name__ == "__main__":
    main()
