"""在已验收的 V03 上接 L_alignment（Luxray 冻结嵌入）。不改 V02 图。"""

from __future__ import annotations

import json
import sys
from collections import defaultdict
from pathlib import Path

import torch

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_embedding.losses import total_loss
from orchemind_embedding.model import ResidualRotate
from orchemind_embedding.train_v03 import (
    CLUSTER_BY_CODE,
    DISPLAY,
    OPERATOR,
    circ_delta,
    fidelity_loss,
    load_json,
    load_triples,
)

TARGETS = ROOT / "data" / "rcc8" / "llm_verb_targets.json"
OUT_JSON = ROOT / "data" / "ontology_graph" / "v03_embedding.json"
SNAPSHOT = ROOT / "snapshot" / "v03_embedding_base" / "v03_embedding.json"
CKPT = ROOT / "data" / "sandbox" / "v03" / "model.pt"
ALIGN_CKPT = ROOT / "data" / "sandbox" / "v03" / "model_align.pt"


def load_targets(codes: list[str], device: torch.device) -> tuple[torch.Tensor, dict]:
    if not TARGETS.exists():
        raise SystemExit("缺少 llm_verb_targets.json，请先运行 fetch_llm_targets.py")
    blob = load_json(TARGETS)
    by_code = {item["code"]: item for item in blob["verbs"]}
    rows = [by_code[code]["embedding"] for code in codes]
    return torch.tensor(rows, dtype=torch.float32, device=device), blob


def main() -> None:
    torch.manual_seed(42)
    if not CKPT.exists():
        raise SystemExit("缺少 V03 权重，请先运行 train_v03.py")

    display = load_json(DISPLAY)
    operator = load_json(OPERATOR)
    display_by_code = {item["code"]: item for item in display["verbs"]}
    op_by_code = {item["code"]: item["deg"] for item in operator["verbs"]}
    codes = [item["code"] for item in display["verbs"]]
    prior = torch.tensor([float(op_by_code[code]) for code in codes], dtype=torch.float32)
    entities, triples = load_triples(codes)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    llm_targets, target_meta = load_targets(codes, device)

    blob = torch.load(CKPT, map_location="cpu", weights_only=False)
    max_delta_deg = float(blob.get("max_delta_deg", 45.0))
    dim = int(blob.get("dim", 32))
    model = ResidualRotate(
        len(entities),
        prior.to(device),
        dim=dim,
        max_delta_deg=max_delta_deg,
        llm_dim=int(target_meta["dim"]),
    ).to(device)
    model.entity.load_state_dict(blob["entity"])
    model.delta.data.copy_(blob["delta"].to(device))
    assert model.w_align is not None

    cluster_ids = torch.tensor(
        [CLUSTER_BY_CODE[code] for code in codes], dtype=torch.long, device=device
    )
    prior_dev = prior.to(device)
    heads = torch.tensor([item[0] for item in triples], device=device)
    rels = torch.tensor([item[1] for item in triples], device=device)
    tails = torch.tensor([item[2] for item in triples], device=device)
    true_tails: dict[tuple[int, int], set[int]] = defaultdict(set)
    for h, r, t in triples:
        true_tails[(h, r)].add(t)
    mask = torch.zeros(len(triples), len(entities), dtype=torch.bool, device=device)
    for index, (h, r, _) in enumerate(triples):
        for t in true_tails[(h, r)]:
            mask[index, t] = True

    history: list[dict] = []
    w_fidelity = 4.0
    w_align = 0.35

    def cosine_report() -> list[float]:
        with torch.no_grad():
            proj = model.projected_verb_vec()
            tgt = torch.nn.functional.normalize(llm_targets, dim=-1)
            return (proj * tgt).sum(dim=-1).detach().cpu().tolist()

    before = cosine_report()
    optimizer = torch.optim.Adam(
        list(model.parameters()),
        lr=5e-3,
    )
    for epoch in range(1, 801):
        optimizer.zero_grad()
        loss, parts = total_loss(
            model,
            heads,
            rels,
            tails,
            cluster_ids,
            true_tail_mask=mask,
            llm_targets=llm_targets,
            w_align=w_align,
        )
        l_fid = fidelity_loss(model, prior_dev)
        total = loss + w_fidelity * l_fid
        total.backward()
        optimizer.step()
        if epoch == 1 or epoch % 200 == 0 or epoch == 800:
            row = {
                "epoch": epoch,
                "phase": "align",
                **parts,
                "fidelity": float(l_fid.detach()),
                "mean_cos": float(sum(cosine_report()) / len(codes)),
            }
            history.append(row)
            print(
                f"[align] {epoch:4d}  task={parts['task']:.4f}  "
                f"align={parts['align']:.4f}  cos={row['mean_cos']:.3f}"
            )

    after = cosine_report()
    learned = model.display_deg().detach().cpu().tolist()
    prior_list = prior.tolist()
    export = {
        "version": "v03_embedding_base",
        "device": str(device),
        "harmonic": 0,
        "label": "第一个 sin（算子角，含 Luxray 对齐）",
        "coefficient": "10000^0 = 1",
        "note": "实心点是旋转算子角；空心点是 angles_f0。L_alignment 对齐 Luxray nomic 嵌入，不微调大模型。",
        "alignment": {
            "provider": target_meta.get("provider"),
            "model": target_meta.get("model"),
            "llm_dim": target_meta.get("dim"),
            "w_align": w_align,
            "cosine_before": before,
            "cosine_after": after,
            "mean_cosine_before": float(sum(before) / len(before)),
            "mean_cosine_after": float(sum(after) / len(after)),
        },
        "entities": entities,
        "n_train_triples": len(triples),
        "max_delta_deg": max_delta_deg,
        "history": history,
        "verbs": [
            {
                "name": display_by_code[code]["name"],
                "code": code,
                "prior_deg": display_by_code[code]["deg"],
                "operator_prior_deg": prior_list[index],
                "learned_deg": learned[index],
                "deg": learned[index],
                "delta_deg": circ_delta(learned[index], prior_list[index]),
                "align_cosine": after[index],
            }
            for index, code in enumerate(codes)
        ],
    }
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    SNAPSHOT.parent.mkdir(parents=True, exist_ok=True)
    ALIGN_CKPT.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(export, ensure_ascii=False, indent=2)
    OUT_JSON.write_text(text, encoding="utf-8")
    SNAPSHOT.write_text(text, encoding="utf-8")
    torch.save(
        {
            "entity": model.entity.state_dict(),
            "delta": model.delta.detach().cpu(),
            "w_align": model.w_align.state_dict() if model.w_align is not None else None,
            "prior_deg": prior,
            "entities": entities,
            "codes": codes,
            "dim": model.dim,
            "max_delta_deg": max_delta_deg,
            "llm_dim": int(target_meta["dim"]),
        },
        ALIGN_CKPT,
    )
    # 评估仍读 CKPT；把对齐后的实体/δ写回主权重，便于 eval
    torch.save(
        {
            "entity": model.entity.state_dict(),
            "delta": model.delta.detach().cpu(),
            "prior_deg": prior,
            "entities": entities,
            "codes": codes,
            "dim": model.dim,
            "max_delta_deg": max_delta_deg,
        },
        CKPT,
    )
    print(
        f"已写入 {OUT_JSON}；对齐余弦 "
        f"{export['alignment']['mean_cosine_before']:.3f} → "
        f"{export['alignment']['mean_cosine_after']:.3f}"
    )


if __name__ == "__main__":
    main()
