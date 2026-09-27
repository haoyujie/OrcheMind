"""Luxray OpenAI 兼容网关：嵌入与短对话。密钥只读环境变量，不写进仓库。"""

from __future__ import annotations

import json
import os
import urllib.error
import urllib.request
from typing import Any


DEFAULT_BASE = "http://ai.luxray.hk.cn/v1"
DEFAULT_EMBED_MODEL = "text-embedding-nomic-embed-text-v1.5"
DEFAULT_CHAT_MODEL = "qwen3-coder-next"


def _headers(api_key: str) -> dict[str, str]:
    return {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {api_key}",
    }


def _post(url: str, payload: dict[str, Any], api_key: str, timeout: float = 120.0) -> dict:
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(url, data=data, headers=_headers(api_key), method="POST")
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"Luxray HTTP {exc.code}: {body}") from exc


def luxray_config() -> tuple[str, str, str, str]:
    base = os.environ.get("LUXRAY_API_BASE", DEFAULT_BASE).rstrip("/")
    key = os.environ.get("LUXRAY_API_KEY", "luxray")
    embed = os.environ.get("LUXRAY_EMBED_MODEL", DEFAULT_EMBED_MODEL)
    chat = os.environ.get("LUXRAY_MODEL", DEFAULT_CHAT_MODEL)
    return base, key, embed, chat


def embed_texts(texts: list[str]) -> list[list[float]]:
    base, key, model, _ = luxray_config()
    out: list[list[float]] = []
    # 一次一条，避免网关对批量限制
    for text in texts:
        blob = _post(
            f"{base}/embeddings",
            {"model": model, "input": text},
            key,
        )
        items = sorted(blob["data"], key=lambda row: int(row["index"]))
        out.append(items[0]["embedding"])
    return out


def chat_once(prompt: str, max_tokens: int = 64) -> str:
    base, key, _, model = luxray_config()
    blob = _post(
        f"{base}/chat/completions",
        {
            "model": model,
            "messages": [{"role": "user", "content": prompt}],
            "temperature": 0.0,
            "max_tokens": max_tokens,
        },
        key,
    )
    return str(blob["choices"][0]["message"]["content"]).strip()
