"""残差复平面旋转：θ = prior + clip(δ)，实体向量经旋转后逼近尾实体。"""

from __future__ import annotations

import math

import torch
from torch import nn


class ResidualRotate(nn.Module):
    def __init__(
        self,
        n_entities: int,
        prior_deg: torch.Tensor,
        dim: int = 8,
        max_delta_deg: float = 6.0,
        llm_dim: int | None = None,
    ) -> None:
        super().__init__()
        if prior_deg.ndim != 1:
            raise ValueError("prior_deg 必须是一维")
        self.n_relations = int(prior_deg.numel())
        self.dim = dim
        self.max_delta = math.radians(max_delta_deg)
        self.register_buffer("prior_rad", prior_deg.deg2rad())
        self.entity = nn.Embedding(n_entities, dim * 2)
        self.delta = nn.Parameter(torch.zeros(self.n_relations, dim))
        nn.init.normal_(self.entity.weight, std=0.05)
        self.w_align: nn.Linear | None = None
        if llm_dim is not None:
            self.attach_align(llm_dim)

    def attach_align(self, llm_dim: int) -> nn.Linear:
        layer = nn.Linear(self.dim * 2, llm_dim, bias=False)
        nn.init.xavier_uniform_(layer.weight)
        self.w_align = layer
        return layer

    def ontology_verb_vec(self) -> torch.Tensor:
        phase = self.relation_phase()
        return nn.functional.normalize(
            torch.cat([phase.cos(), phase.sin()], dim=-1), dim=-1
        )

    def projected_verb_vec(self) -> torch.Tensor:
        if self.w_align is None:
            raise RuntimeError("尚未 attach_align")
        return nn.functional.normalize(self.w_align(self.ontology_verb_vec()), dim=-1)

    def relation_phase(self) -> torch.Tensor:
        return self.prior_rad.unsqueeze(1) + self.max_delta * torch.tanh(self.delta)

    def display_deg(self) -> torch.Tensor:
        phase = self.relation_phase()
        mean_cos = phase.cos().mean(dim=1)
        mean_sin = phase.sin().mean(dim=1)
        deg = torch.atan2(mean_sin, mean_cos).rad2deg()
        return (deg + 360.0) % 360.0

    def rotate(self, head: torch.Tensor, rel_index: torch.Tensor) -> torch.Tensor:
        phase = self.relation_phase()[rel_index]
        real, imag = head[:, : self.dim], head[:, self.dim :]
        cos_p, sin_p = phase.cos(), phase.sin()
        out_real = real * cos_p - imag * sin_p
        out_imag = real * sin_p + imag * cos_p
        return torch.cat([out_real, out_imag], dim=-1)

    def entity_vec(self, index: torch.Tensor) -> torch.Tensor:
        return nn.functional.normalize(self.entity(index), dim=-1)

    def score(
        self, heads: torch.Tensor, rels: torch.Tensor, tails: torch.Tensor
    ) -> torch.Tensor:
        rotated = self.rotate(self.entity_vec(heads), rels)
        tail_vec = self.entity_vec(tails)
        return ((rotated - tail_vec) ** 2).sum(dim=-1)

    def forward(
        self, heads: torch.Tensor, rels: torch.Tensor, tails: torch.Tensor
    ) -> torch.Tensor:
        return self.score(heads, rels, tails).mean()
