"""在仓库根目录起一个本地页面，用来看本体地图和第一谐波关系圆。"""

from __future__ import annotations

import argparse
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8765)
    args = parser.parse_args()
    handler = lambda *a, **k: SimpleHTTPRequestHandler(*a, directory=str(ROOT), **k)
    server = ThreadingHTTPServer((args.host, args.port), handler)
    print(f"本体视图：http://{args.host}:{args.port}/view/index.html")
    server.serve_forever()


if __name__ == "__main__":
    main()
