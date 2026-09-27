"""从 Luxray 拉取 RCC8 动词的 LLM 嵌入目标，写入可入库的小 JSON。"""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "src") not in sys.path:
    sys.path.insert(0, str(ROOT / "src"))

from orchemind_embedding.luxray_client import embed_texts, luxray_config

RELATIONS = ROOT / "data" / "rcc8" / "relations.json"
OUT = ROOT / "data" / "rcc8" / "llm_verb_targets.json"
CACHE = ROOT / "data" / "llm_cache" / "luxray_verb_embeddings.json"


def main() -> None:
    relations = json.loads(RELATIONS.read_text(encoding="utf-8"))["relations"]
    prompts: list[str] = []
    meta: list[dict] = []
    for item in relations:
        text = (
            f"RCC8 relation {item['code']}: {item['name_en']}; "
            f"中文「{item['name_zh']}」；inverse={item['inverse']}"
        )
        prompts.append(text)
        meta.append(
            {
                "code": item["code"],
                "name_zh": item["name_zh"],
                "name_en": item["name_en"],
                "prompt": text,
            }
        )

    base, _, model, _ = luxray_config()
    print(f"拉取嵌入：{model} @ {base}，共 {len(prompts)} 条")
    vectors = embed_texts(prompts)
    export = {
        "provider": "luxray",
        "base_url": base,
        "model": model,
        "dim": len(vectors[0]),
        "note": "本体 L_alignment 的冻结目标向量；不微调大模型，只读 embedding。",
        "verbs": [
            {**meta[index], "embedding": vectors[index]} for index in range(len(meta))
        ],
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    CACHE.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(export, ensure_ascii=False, indent=2)
    OUT.write_text(text, encoding="utf-8")
    CACHE.write_text(text, encoding="utf-8")
    print(f"已写入 {OUT}（dim={export['dim']}）")


if __name__ == "__main__":
    main()
