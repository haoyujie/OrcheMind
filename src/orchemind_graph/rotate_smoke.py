"""GPU 冒烟：在 4060 上用抽象三元组跑几步复平面旋转，不写回本体图。"""

from __future__ import annotations

import json
import sys
from pathlib import Path

import torch

ROOT = Path(__file__).resolve().parents[2]
TRIPLES = ROOT / "data" / "triples" / "rcc8_abstract.jsonl"
OUT_DIR = ROOT / "data" / "sandbox" / "rotate_smoke"


def load_triples() -> list[tuple[str, str, str]]:
    rows = []
    for line in TRIPLES.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        item = json.loads(line)
        rows.append((item["head"], item["relation"], item["tail"]))
    return rows


def main() -> None:
    if not torch.cuda.is_available():
        print("CUDA 不可用")
        raise SystemExit(1)
    device = torch.device("cuda")
    triples = load_triples()
    entities = sorted({head for head, _, _ in triples} | {tail for _, _, tail in triples})
    relations = sorted({relation for _, relation, _ in triples})
    entity_index = {name: index for index, name in enumerate(entities)}
    relation_index = {name: index for index, name in enumerate(relations)}
    heads = torch.tensor([entity_index[head] for head, _, _ in triples], device=device)
    rels = torch.tensor([relation_index[relation] for _, relation, _ in triples], device=device)
    tails = torch.tensor([entity_index[tail] for _, _, tail in triples], device=device)

    dim = 16
    entity = torch.nn.Embedding(len(entities), dim * 2).to(device)
    relation = torch.nn.Embedding(len(relations), dim).to(device)
    optimizer = torch.optim.Adam(list(entity.parameters()) + list(relation.parameters()), lr=1e-2)
    losses: list[float] = []
    for _ in range(40):
        optimizer.zero_grad()
        head_vec = entity(heads)
        tail_vec = entity(tails)
        phase = relation(rels)
        head_real, head_imag = head_vec[:, :dim], head_vec[:, dim:]
        rotation_real, rotation_imag = torch.cos(phase), torch.sin(phase)
        rotated_real = head_real * rotation_real - head_imag * rotation_imag
        rotated_imag = head_real * rotation_imag + head_imag * rotation_real
        tail_real, tail_imag = tail_vec[:, :dim], tail_vec[:, dim:]
        loss = ((rotated_real - tail_real) ** 2 + (rotated_imag - tail_imag) ** 2).mean()
        loss.backward()
        optimizer.step()
        losses.append(float(loss.detach().cpu()))

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    result = {
        "torch": torch.__version__,
        "cuda": torch.version.cuda,
        "device": torch.cuda.get_device_name(0),
        "triples": len(triples),
        "loss_first": losses[0],
        "loss_last": losses[-1],
        "note": "沙箱旋转冒烟，权重不进入本体图，也不是 WN18RR 预训练。",
    }
    (OUT_DIR / "result.json").write_text(
        json.dumps(result, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    print(json.dumps(result, ensure_ascii=False))


if __name__ == "__main__":
    main()
