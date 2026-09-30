"""第一步：实体目录候选扫描（C# 类型 + ODAAF 规则节点）。

只做一件事：产出"主语/宾语"候选全集，可数、可查漏、可人工复核。
- C# 类型：正则扫描（class/interface/enum/struct/record），带 namespace + 出处 file:line。
- ODAAF 规则节点：XML 解析（Class + DataProperty），带权威 OID。
- 不归一化、不发明 OID、不生成三元组。归一化交给 LLM（--prompt-only 输出提示词模板）。

铁律：产物只进领域层 project_dna；不改 RCC8 基础层；不生成三元组。
用法（示例）：
  python src\\orchemind_graph\\scan_project_entities.py ^
    --root "G:\\work\\main\\dev\\dev-01-develop\\OmronXmlGenerator" ^
    --odaaf "G:\\work\\main\\dev\\dev-01-develop\\OmronXmlGenerator\\ODAAF\\bzox\\zextract-rules.odaaf" ^
    --out data\\project_dna\\omron-zextract\\v003\\entity_candidates.json
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
ODAAF_NS = "{urn:odaaf:project}"

# C# 类型声明（行首、可带修饰符与特性之间的换行忽略，先抓单行声明）
TYPE_RE = re.compile(
    r"^\s*(?:(?:public|internal|private|protected|file)\s+)?"
    r"(?:static\s+|abstract\s+|sealed\s+|partial\s+|readonly\s+|unsafe\s+)*"
    r"(class|interface|struct|enum|record)\s+([A-Za-z_]\w*)",
    re.MULTILINE,
)
NS_RE = re.compile(r"^\s*namespace\s+([\w\.]+)\s*", re.MULTILINE)

# 跳过目录（全小写匹配）
SKIP_DIRS = {
    "bin", "obj", "packages", ".vs", ".git", "tmp", "temp", "logs",
    "out", "node_modules", "dist", ".cr", "issuedata", ".specstory",
    ".cursor", "obfuscar", "data", "resource", "resources", "policies",
}


def skip_dir(path: Path) -> bool:
    for part in path.parts:
        if part.lower() in SKIP_DIRS:
            return True
    return False


def snake(name: str) -> str:
    s = re.sub(r"(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])", "_", name)
    return s.lower()


def scan_csharp(root: Path) -> tuple[list[dict], int]:
    """扫描全部 .cs，返回候选类型列表与文件数。"""
    candidates: list[dict] = []
    files = 0
    for path in sorted(root.rglob("*.cs")):
        if skip_dir(path):
            continue
        files += 1
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError as exc:
            print(f"  [跳过] 读取失败 {path}: {exc}", file=sys.stderr)
            continue
        rel = path.relative_to(root).as_posix()
        # 当前文件的 namespace（就近原则：最后一次出现的 namespace）
        ns_matches = NS_RE.findall(text)
        namespace = ns_matches[-1] if ns_matches else ""
        for m in TYPE_RE.finditer(text):
            kind, name = m.group(1), m.group(2)
            line = text.count("\n", 0, m.start()) + 1
            candidates.append({
                "id": f"zx.cs.{snake(name)}",
                "name": name,
                "kind": kind,
                "namespace": namespace,
                "file": rel,
                "line": line,
                "aliases": [],
            })
        # 文件名兜底：C# 惯例 类型名.cs；若该名未出现在类型声明里，作为候选补漏
        stem = path.stem
        if stem and not any(c["name"] == stem for c in candidates if c["file"] == rel):
            candidates.append({
                "id": f"zx.cs.{snake(stem)}",
                "name": stem,
                "kind": "file",
                "namespace": namespace,
                "file": rel,
                "line": 1,
                "aliases": [],
                "note": "来自文件名，未在声明中找到同名类型",
            })
    # 按 (file, line) 去重
    seen: set[tuple[str, str, int]] = set()
    unique: list[dict] = []
    for c in candidates:
        key = (c["file"], c["name"], c["line"])
        if key in seen:
            continue
        seen.add(key)
        unique.append(c)
    return unique, files


def parse_odaaf(path: Path) -> list[dict]:
    """解析 ODAAF XML：Class（实体节点）+ DataProperty（属性，带 OID）。"""
    tree = ET.parse(path)
    root = tree.getroot()
    nodes: list[dict] = []
    for cls in root.iter(f"{ODAAF_NS}Class"):
        props: list[dict] = []
        for dp in cls.findall(f"{ODAAF_NS}DataProperties/{ODAAF_NS}DataProperty"):
            props.append({
                "name": dp.get("name", ""),
                "oid": dp.get("oid", ""),
                "type": dp.get("dataType", ""),
                "label": dp.get("label", ""),
            })
        nodes.append({
            "id": f"zx.{snake(cls.get('name', ''))}",
            "oid": cls.get("oid", ""),
            "name": cls.get("name", ""),
            "label": cls.get("label", ""),
            "machine_name": cls.get("machineName", ""),
            "kind": cls.get("category", ""),
            "data_properties": props,
            "aliases": [],
            "note": "ODAAF 权威节点，OID 唯一",
        })
    return nodes


NORMALIZE_PROMPT = """你是本体工程师。下面是候选实体目录（JSON）。
任务：只做实体归一化合并，禁止创建关系，禁止发明 OID。

规则：
1. 同义合并：C# 候选与 ODAAF 权威节点若指同一概念（如 C# 的 Pad / Land 与规则节点的焊盘），合并到 ODAAF 权威节点（保留其 OID），不新建。
2. 纯 C# 新实体：用 zx.cs.<snake_name> 命名；OID 只属于 ODAAF 节点，C# 实体一律不给 OID。
3. aliases 列同义词（含中文、英文、变体名）。
4. confidence ∈ [0,1]，给出理由一句话。
5. 只输出 JSON 数组，每个元素：
   {{"id": "...", "name": "...", "kind": "class|interface|enum|struct|record|rule_node", "oid": "（有则填，无则省略）", "aliases": [...], "source_kinds": ["cs","odaaf"], "confidence": 0.0-1.0, "reason": "..."}}
不要输出 JSON 以外的任何文字。
"""


def build_prompt(cs_candidates: list[dict], odaaf_nodes: list[dict]) -> str:
    payload = {
        "instruction": "合并候选实体到闭合实体目录。",
        "odaaf_authoritative": [{"id": n["id"], "oid": n["oid"], "name": n["name"], "label": n["label"]} for n in odaaf_nodes],
        "cs_candidates": [
            {"id": c["id"], "name": c["name"], "kind": c["kind"], "namespace": c["namespace"], "file": c["file"], "line": c["line"]}
            for c in cs_candidates
        ],
    }
    return NORMALIZE_PROMPT + "\n\n" + json.dumps(payload, ensure_ascii=False, indent=2)


def main() -> None:
    ap = argparse.ArgumentParser(description="实体目录候选扫描（第一步）")
    ap.add_argument("--root", required=True, help="目标工程根目录（含 src/）")
    ap.add_argument("--odaaf", default="", help="zextract-rules.odaaf 路径（可选）")
    ap.add_argument("--out", default="data/project_dna/omron-zextract/v003/entity_candidates.json")
    ap.add_argument("--prompt-only", action="store_true", help="只输出 LLM 归一化提示词，不写文件")
    args = ap.parse_args()

    root = Path(args.root)
    if not root.is_dir():
        raise SystemExit(f"工程根不存在: {root}")

    cs_candidates, cs_files = scan_csharp(root)
    odaaf_nodes = parse_odaaf(Path(args.odaaf)) if args.odaaf else []

    prompt = build_prompt(cs_candidates, odaaf_nodes)
    if args.prompt_only:
        print(prompt)
        return

    out = ROOT / args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "project": "omron-zextract",
        "dna_version": "v003-candidates",
        "note": "第一步实体目录候选：C# 类型（无 OID）+ ODAAF 规则节点（有 OID）。未归一化、未定 OID、未人工复核、无三元组。",
        "scan": {
            "root": str(root),
            "cs_files": cs_files,
            "cs_candidates": len(cs_candidates),
            "odaaf_nodes": len(odaaf_nodes),
            "cs_skipped_dirs": sorted(SKIP_DIRS),
        },
        "cs_candidates": cs_candidates,
        "odaaf_nodes": odaaf_nodes,
        "normalize_prompt_file": "normalize_prompt.txt",
    }
    out.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    prompt_path = out.parent / "normalize_prompt.txt"
    prompt_path.write_text(prompt, encoding="utf-8")

    # 摘要
    kinds: dict[str, int] = {}
    for c in cs_candidates:
        kinds[c["kind"]] = kinds.get(c["kind"], 0) + 1
    by_ns: dict[str, int] = {}
    for c in cs_candidates:
        ns = c["namespace"] or "(global)"
        by_ns[ns] = by_ns.get(ns, 0) + 1
    print(f"C# 文件数: {cs_files}  候选类型: {len(cs_candidates)}")
    print("kind 分布:", dict(sorted(kinds.items())))
    print("namespace 前 12:", dict(sorted(by_ns.items(), key=lambda kv: -kv[1])[:12]))
    if odaaf_nodes:
        print(f"ODAAF 规则节点: {len(odaaf_nodes)}")
        print("  " + ", ".join(f"{n['name']}({n['oid'].rsplit('.',1)[-1]})" for n in odaaf_nodes))
    print(f"已写入 {out}")
    print(f"提示词已写入 {prompt_path}")


if __name__ == "__main__":
    main()
