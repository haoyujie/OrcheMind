"""与 OID 并列的声明谐波坐标。第 0 谐波是动词先验角，更高谐波写入 OID。

这不是 V03 的检索。未见过 orcheVerb 的图无法反查簇。
"""

from __future__ import annotations

import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DNA = ROOT / "data" / "project_dna"
PRIORS = DNA / "harmonic_priors.json"
VERBS = DNA / "verbs.json"
DIM = 8


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def pack_oid(oid: str) -> int:
    parts = [int(piece) for piece in oid.split(".")]
    if 862 not in parts and 55555 not in parts:
        raise ValueError(f"OID 缺少企业弧: {oid}")
    anchor = 862 if 862 in parts else 55555
    suffix = parts[parts.index(anchor) + 1 :]
    packed = 0
    for piece in suffix:
        packed = packed * 100 + piece
    return packed


def phase(theta_deg: float, oid: str, harmonic: int) -> float:
    alpha = pack_oid(oid) / 1_000_000.0
    return math.radians(theta_deg) + 2.0 * math.pi * alpha * harmonic


def vector(theta_deg: float, oid: str, dim: int = DIM) -> list[float]:
    coords: list[float] = []
    for harmonic in range(dim):
        phi = phase(theta_deg, oid, harmonic)
        coords.append(round(math.cos(phi), 10))
        coords.append(round(math.sin(phi), 10))
    return coords


def nearest_verb(coords: list[float], priors: dict) -> tuple[str, float]:
    angle = math.atan2(coords[1], coords[0])
    best_code = ""
    best_delta = 360.0
    for item in priors["verbs"]:
        delta = abs((math.degrees(angle) - item["deg"] + 180.0) % 360.0 - 180.0)
        if delta < best_delta:
            best_delta = delta
            best_code = item["code"]
    return best_code, best_delta


def nearest_node(coords: list[float], table: list[dict]) -> dict:
    best = table[0]
    best_dist = 1e9
    for item in table:
        dist = sum((a - b) ** 2 for a, b in zip(coords, item["coord"])) ** 0.5
        if dist < best_dist:
            best_dist = dist
            best = item
    return best


def build(version_dir: Path, sidecar: Path | None = None) -> dict:
    ontology = load(version_dir / "ontology.json")
    priors = load(PRIORS)
    verbs = load(VERBS)
    by_deg = {item["code"]: item["deg"] for item in priors["verbs"]}
    by_cluster = {item["code"]: item["cluster"] for item in verbs["verbs"]}
    rows = []
    for node in ontology["nodes"]:
        code = node["anchor_verb"]
        if code not in by_deg:
            raise SystemExit(f"{node['id']} 的锚动词 {code} 没有先验角")
        coord = vector(by_deg[code], node["oid"])
        recovered, delta = nearest_verb(coord, priors)
        if recovered != code or delta > 1e-3:
            raise SystemExit(f"{node['id']} 第 0 谐波没有回到 {code}（得到 {recovered}，{delta}°）")
        rows.append(
            {
                "id": node["id"],
                "oid": node["oid"],
                "anchor_verb": code,
                "cluster": by_cluster[code],
                "theta_deg": by_deg[code],
                "coord": coord,
            }
        )
    for row in rows:
        hit = nearest_node(row["coord"], rows)
        if hit["id"] != row["id"]:
            raise SystemExit(f"{row['id']} 的全向量最近邻不是它自己，而是 {hit['id']}")
    payload = {
        "project": ontology["project"],
        "dna_version": ontology["dna_version"],
        "dim": DIM,
        "note": "第 0 谐波 = 锚动词先验角的 cos/sin。更高谐波加上 OID 打包值，用来在同动词节点之间分开。声明坐标，不是训练嵌入，也不是向量库。",
        "nodes": rows,
    }
    out = version_dir / "coordinates.json"
    out.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if sidecar is not None:
        sidecar.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return payload


def main() -> None:
    import sys

    version_name = sys.argv[1] if len(sys.argv) > 1 else "v002"
    version = DNA / "omron-zextract" / version_name
    sidecar = None
    if version_name == "v002":
        sidecar = Path(
            r"g:\work\main\dev\dev-01-develop\OmronXmlGenerator\ODAAF\bzox\zextract-harmonic.coord.json"
        )
    payload = build(version, sidecar)
    print(f"坐标已写入 {len(payload['nodes'])} 个节点 ({version_name})")
    for row in payload["nodes"]:
        print(f"{row['id']}\t{row['cluster']}\t{row['anchor_verb']}\t{row['theta_deg']}")


if __name__ == "__main__":
    main()
