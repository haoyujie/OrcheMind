"""在抽象 RCC8 三元组上训练 V03。展示角与旋转算子角分离；不改 V02 图。"""

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

DISPLAY = ROOT / "data" / "rcc8" / "angles_f0.json"

OPERATOR = ROOT / "data" / "rcc8" / "operator_prior.json"

RELATIONS = ROOT / "data" / "rcc8" / "relations.json"

TRIPLES = ROOT / "data" / "triples" / "rcc8_abstract.jsonl"

OUT_JSON = ROOT / "data" / "ontology_graph" / "v03_embedding.json"

SNAPSHOT = ROOT / "snapshot" / "v03_embedding_base" / "v03_embedding.json"

CKPT = ROOT / "data" / "sandbox" / "v03" / "model.pt"

CLUSTER_BY_CODE = {

    "DC": 0,

    "EC": 0,

    "PO": 0,

    "EQ": 0,

    "TPP": 1,

    "NTPP": 1,

    "TPPi": 2,

    "NTPPi": 2,

}

def load_json(path: Path) -> dict:

    return json.loads(path.read_text(encoding="utf-8"))

def load_triples(relation_codes: list[str]) -> tuple[list[str], list[tuple[int, int, int]]]:

    code_index = {code: index for index, code in enumerate(relation_codes)}

    inv_map = {

        item["code"]: item["inverse"]

        for item in load_json(RELATIONS)["relations"]

    }

    entities: list[str] = []

    entity_index: dict[str, int] = {}

    seen: set[tuple[int, int, int]] = set()

    triples: list[tuple[int, int, int]] = []

    def add_entity(name: str) -> int:

        if name not in entity_index:

            entity_index[name] = len(entities)

            entities.append(name)

        return entity_index[name]

    def add_triple(h: int, r: int, t: int) -> None:

        key = (h, r, t)

        if key in seen:

            return

        seen.add(key)

        triples.append(key)

    for line in TRIPLES.read_text(encoding="utf-8").splitlines():

        if not line.strip():

            continue

        item = json.loads(line)

        h = add_entity(item["head"])

        t = add_entity(item["tail"])

        rel = item["relation"]

        add_triple(h, code_index[rel], t)

        inv = inv_map[rel]

        if inv != rel:

            add_triple(t, code_index[inv], h)

    return entities, triples

def circ_delta(a: float, b: float) -> float:

    return (a - b + 180.0) % 360.0 - 180.0

def fidelity_loss(model: ResidualRotate, prior_deg: torch.Tensor) -> torch.Tensor:

    """把展示角（各维圆周均值）拉回算子先验。"""

    learned = model.display_deg()

    delta = (learned - prior_deg + 180.0) % 360.0 - 180.0

    return (delta / 180.0).pow(2).mean()

def main() -> None:

    torch.manual_seed(42)

    display = load_json(DISPLAY)

    operator = load_json(OPERATOR)

    display_by_code = {item["code"]: item for item in display["verbs"]}

    op_by_code = {item["code"]: item["deg"] for item in operator["verbs"]}

    codes = [item["code"] for item in display["verbs"]]

    prior = torch.tensor([float(op_by_code[code]) for code in codes], dtype=torch.float32)

    entities, triples = load_triples(codes)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # 各维允许更大散开；展示角用 fidelity 拉回先验

    max_delta_deg = 45.0

    model = ResidualRotate(

        len(entities), prior.to(device), dim=32, max_delta_deg=max_delta_deg

    ).to(device)

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

    def run_phase(epochs: int, lr: float, tag: str) -> None:

        optimizer = torch.optim.Adam(

            [model.entity.weight, model.delta], lr=lr

        )

        for epoch in range(1, epochs + 1):

            optimizer.zero_grad()

            loss, parts = total_loss(

                model, heads, rels, tails, cluster_ids, true_tail_mask=mask

            )

            l_fid = fidelity_loss(model, prior_dev)

            total = loss + w_fidelity * l_fid

            total.backward()

            optimizer.step()

            if epoch == 1 or epoch % 200 == 0 or epoch == epochs:

                row = {

                    "epoch": epoch,

                    "phase": tag,

                    **parts,

                    "fidelity": float(l_fid.detach()),

                    "total_with_fid": float(total.detach()),

                }

                history.append(row)

                print(

                    f"[{tag}] {epoch:4d}  task={parts['task']:.4f}  "

                    f"fid={row['fidelity']:.4f}  total={row['total_with_fid']:.4f}"

                )

    run_phase(2000, lr=2e-2, tag="joint")

    run_phase(1000, lr=5e-3, tag="refine")

    learned = model.display_deg().detach().cpu().tolist()

    prior_list = prior.tolist()

    export = {

        "version": "v03_embedding_base",

        "device": str(device),

        "harmonic": 0,

        "label": "第一个 sin（算子角，训练后）",

        "coefficient": "10000^0 = 1",

        "note": "实心点是旋转算子角（EQ≈0，逆对约差180°）；空心点是 angles_f0 语义扇区。不改 V02。",

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

            }

            for index, code in enumerate(codes)

        ],

    }

    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)

    SNAPSHOT.parent.mkdir(parents=True, exist_ok=True)

    CKPT.parent.mkdir(parents=True, exist_ok=True)

    text = json.dumps(export, ensure_ascii=False, indent=2)

    OUT_JSON.write_text(text, encoding="utf-8")

    SNAPSHOT.write_text(text, encoding="utf-8")

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

    drifts = [abs(circ_delta(learned[i], prior_list[i])) for i in range(len(codes))]

    print(

        f"已写入 {OUT_JSON}（三元组 {len(triples)}，"

        f"最大漂移 {max(drifts):.2f}°）"

    )

if __name__ == "__main__":

    main()

