"""V02：在 V01 的节点和连线身份上增加类型、方向和端口。"""

from __future__ import annotations

import uuid
from pathlib import Path

from pydantic import BaseModel, Field


class Port(BaseModel):
    """节点上按关系类型分化的接口。"""

    id: str
    link_type: str
    role: str


class TypedNode(BaseModel):
    id: str
    name: str
    ports: list[Port] = Field(default_factory=list)


class TypedLink(BaseModel):
    id: str
    source_id: str
    target_id: str
    link_type: str
    directed: bool


class TypedOntologyGraph(BaseModel):
    nodes: list[TypedNode] = Field(default_factory=list)
    links: list[TypedLink] = Field(default_factory=list)

    def save_json(self, path: str | Path) -> None:
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(self.model_dump_json(indent=2), encoding="utf-8")

    @classmethod
    def load_json(cls, path: str | Path) -> TypedOntologyGraph:
        return cls.model_validate_json(Path(path).read_text(encoding="utf-8"))


def stable_id(key: str) -> str:
    return str(uuid.uuid5(uuid.NAMESPACE_URL, f"orchemind:v02:{key}"))
