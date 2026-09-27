"""V01 裸图：节点与连线，无端口、无类型、无方向。"""

from __future__ import annotations

import uuid
from pathlib import Path

from pydantic import BaseModel, Field


class Node(BaseModel):
    """裸节点：只有 id 和名称。"""

    id: str = Field(default_factory=lambda: str(uuid.uuid4()))
    name: str


class Link(BaseModel):
    """裸连线：只有源与目标，无类型、无方向。"""

    id: str = Field(default_factory=lambda: str(uuid.uuid4()))
    source_id: str
    target_id: str


class RawOntologyGraph(BaseModel):
    nodes: list[Node] = Field(default_factory=list)
    links: list[Link] = Field(default_factory=list)

    def add_node(self, name: str) -> Node:
        node = Node(name=name)
        self.nodes.append(node)
        return node

    def add_link(self, source_id: str, target_id: str) -> Link:
        link = Link(source_id=source_id, target_id=target_id)
        self.links.append(link)
        return link

    def save_json(self, path: str | Path) -> None:
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(
            self.model_dump_json(indent=2),
            encoding="utf-8",
        )

    @classmethod
    def load_json(cls, path: str | Path) -> RawOntologyGraph:
        return cls.model_validate_json(Path(path).read_text(encoding="utf-8"))
