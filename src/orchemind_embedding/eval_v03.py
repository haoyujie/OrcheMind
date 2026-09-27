"""验收：算子角漂移受控；过滤设定下三元组 Hit@1。"""

from __future__ import annotations

import json
import sys
from collections import defaultdict
from pathlib import Path

import torch

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_embedding.model import ResidualRotate

EMBEDDING = ROOT / "data" / "ontology_graph" / "v03_embedding.json"
CKPT = ROOT / "data" / "sandbox" / "v03" / "model.pt"
TRIPLES = ROOT / "data" / "triples" / "rcc8_abstract.jsonl"


def circ_dist(a: float, b: float) -> float:
    return min((a - b) % 360.0, (b - a) % 360.0)


def main() -> None:
    if not EMBEDDING.exists() or not CKPT.exists():
        print("缺少 v03 导出或权重，请先运行 train_v03.py")
        raise SystemExit(1)

    export = json.loads(EMBEDDING.read_text(encoding="utf-8"))
    max_delta = 8.5
    fidelity_fail = []
    for verb in export["verbs"]:
        drift = circ_dist(verb["learned_deg"], verb["operator_prior_deg"])
        if drift > max_delta:
            fidelity_fail.append(
                f"{verb['code']}: op_prior={verb['operator_prior_deg']} "
                f"learned={verb['learned_deg']:.2f} drift={drift:.2f}"
            )
    print(
        f"算子先验保真：{len(export['verbs']) - len(fidelity_fail)}/"
        f"{len(export['verbs'])}（≤ {max_delta}°）"
    )
    for item in fidelity_fail:
        print("  FAIL", item)

    by_code = {item["code"]: item["learned_deg"] for item in export["verbs"]}
    for left, right in (("TPP", "TPPi"), ("NTPP", "NTPPi")):
        dist = circ_dist(by_code[left], (by_code[right] + 180.0) % 360.0)
        print(f"逆对 {left}/{right} 与 180° 偏差：{dist:.1f}°")

    blob = torch.load(CKPT, map_location="cpu", weights_only=False)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = ResidualRotate(
        len(blob["entities"]),
        blob["prior_deg"].to(device),
        dim=int(blob.get("dim", 16)),
        max_delta_deg=float(blob.get("max_delta_deg", 8.0)),
    ).to(device)
    model.entity.load_state_dict(blob["entity"])
    model.delta.data.copy_(blob["delta"].to(device))
    model.eval()

    entity_index = {name: index for index, name in enumerate(blob["entities"])}
    code_index = {code: index for index, code in enumerate(blob["codes"])}
    true_tails: dict[tuple[int, int], set[int]] = defaultdict(set)
    rows: list[tuple[int, int, int]] = []
    for line in TRIPLES.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        item = json.loads(line)
        h = entity_index[item["head"]]
        r = code_index[item["relation"]]
        t = entity_index[item["tail"]]
        true_tails[(h, r)].add(t)
        rows.append((h, r, t))

    hits = 0
    with torch.no_grad():
        all_norm = torch.nn.functional.normalize(model.entity.weight, dim=-1)
        for h, r, t in rows:
            head = torch.tensor([h], device=device)
            rel = torch.tensor([r], device=device)
            rotated = model.rotate(model.entity_vec(head), rel)
            dist = ((all_norm - rotated) ** 2).sum(dim=-1).clone()
            # 过滤设定：去掉同一 (h,r) 的其他真尾，只保留当前 t
            for other in true_tails[(h, r)]:
                if other != t:
                    dist[other] = float("inf")
            if int(dist.argmin().item()) == t:
                hits += 1

    total = len(rows)
    print(f"过滤 Hit@1：{hits}/{total}")
    min_hits = max(1, int(total * 0.85))
    if fidelity_fail or hits < min_hits:
        raise SystemExit(1)
    print("V03 验收通过")


if __name__ == "__main__":
    main()
