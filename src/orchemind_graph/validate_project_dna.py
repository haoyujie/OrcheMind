"""校验工程 DNA：动词闭合、端口指回节点、簇不得超出清单。"""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DNA = ROOT / "data" / "project_dna"
VERBS = DNA / "verbs.json"
REQUIRED = ("ontology.json", "schema.json", "clusters.json", "code_links.json", "README.md")


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def validate_version(version_dir: Path, verbs: dict) -> list[str]:
    errors: list[str] = []
    for name in REQUIRED:
        if not (version_dir / name).exists():
            errors.append(f"{version_dir.name} 缺少 {name}")
    if errors:
        return errors

    allowed_clusters = {item["id"] for item in verbs["clusters"]}
    by_code = {}
    for item in verbs["verbs"]:
        code = item["code"]
        if code in by_code:
            errors.append(f"动词重复: {code}")
        if item["cluster"] not in allowed_clusters:
            errors.append(f"动词 {code} 使用了未登记簇 {item['cluster']}")
        name = str(item.get("name", "")).strip()
        if not name or not any("A" <= ch <= "Z" or "a" <= ch <= "z" for ch in name):
            errors.append(f"动词 {code} 缺少英文 name")
        by_code[code] = item
    extension = by_code.get("pass_through")
    if extension is None or extension.get("kind") != "extension" or not extension.get("not_rcc8"):
        errors.append("穿过必须登记为扩展动词且 not_rcc8")

    ontology = load(version_dir / "ontology.json")
    schema = load(version_dir / "schema.json")
    clusters = load(version_dir / "clusters.json")
    code_links = load(version_dir / "code_links.json")

    node_ids = [node["id"] for node in ontology["nodes"]]
    if len(node_ids) != len(set(node_ids)):
        errors.append("本体节点 id 重复")
    nodes = set(node_ids)
    link_ids = []
    for link in ontology["links"]:
        link_ids.append(link["id"])
        verb = link["verb"]
        if verb not in by_code:
            errors.append(f"连线 {link['id']} 的动词 {verb} 不在清单内")
        if link["source_id"] not in nodes or link["target_id"] not in nodes:
            errors.append(f"连线 {link['id']} 的端点不在本体节点中")
    if len(link_ids) != len(set(link_ids)):
        errors.append("本体连线 id 重复")

    for port in schema["ports"]:
        if port["node_id"] not in nodes:
            errors.append(f"端口 {port['id']} 没有指回本体节点")
        for verb in port["allowed_verbs"]:
            if verb not in by_code:
                errors.append(f"端口 {port['id']} 允许了清单外动词 {verb}")

    for verb, cluster in clusters.get("verb_clusters", {}).items():
        if cluster not in allowed_clusters:
            errors.append(f"簇文件出现清单外的簇 {cluster}")
        if verb not in by_code:
            errors.append(f"簇文件出现清单外动词 {verb}")
        elif by_code[verb]["cluster"] != cluster:
            errors.append(f"动词 {verb} 的簇与清单不一致")
    link_clusters = clusters.get("link_clusters", {})
    if set(link_clusters) != set(link_ids):
        errors.append("连线簇与本体连线不是同一批")
    for link in ontology["links"]:
        cluster = link_clusters.get(link["id"])
        if cluster not in allowed_clusters:
            errors.append(f"连线 {link['id']} 的簇不在清单内")
        elif link["verb"] in by_code and by_code[link["verb"]]["cluster"] != cluster:
            errors.append(f"连线 {link['id']} 的簇与动词簇不一致")

    for item in code_links["links"]:
        if item["node_id"] not in nodes:
            errors.append(f"代码声明 {item['path']} 没有指回本体节点")
    if code_links.get("dna_version") != ontology.get("dna_version"):
        errors.append("code_links 的版本号与本体不一致")
    return errors


def main() -> None:
    verbs = load(VERBS)
    errors: list[str] = []
    versions = sorted(
        path for path in DNA.glob("*/*")
        if path.is_dir() and (path / "ontology.json").is_file()
    )
    if not versions:
        errors.append("没有工程 DNA 版本目录")
    for version_dir in versions:
        errors.extend(validate_version(version_dir, verbs))
    if errors:
        for item in errors:
            print(item)
        raise SystemExit(1)
    print(f"工程 DNA 验收通过（{len(versions)} 个版本）")


if __name__ == "__main__":
    main()
