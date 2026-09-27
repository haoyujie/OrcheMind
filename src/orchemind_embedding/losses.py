"""四项损失：任务、簇内、簇间、对齐。对齐走 Luxray 冻结嵌入目标。"""

from __future__ import annotations

import torch
from torch import nn

from orchemind_embedding.model import ResidualRotate


def unit_vectors_from_phase(phase: torch.Tensor) -> torch.Tensor:
    vec = torch.cat([phase.cos(), phase.sin()], dim=-1)
    return nn.functional.normalize(vec, dim=-1)


def task_loss(
    model: ResidualRotate,
    heads: torch.Tensor,
    rels: torch.Tensor,
    tails: torch.Tensor,
    true_tail_mask: torch.Tensor | None = None,
    temperature: float = 0.5,
) -> torch.Tensor:
    """过滤式全实体交叉熵：与验收用的过滤 Hit@1 同设定。"""
    batch = heads.size(0)
    rotated = model.rotate(model.entity_vec(heads), rels)
    all_ent = nn.functional.normalize(model.entity.weight, dim=-1)
    logits = -((rotated.unsqueeze(1) - all_ent.unsqueeze(0)) ** 2).sum(dim=-1)
    logits = logits / temperature
    if true_tail_mask is not None:
        other = true_tail_mask.clone()
        other[torch.arange(batch, device=tails.device), tails] = False
        logits = logits.masked_fill(other, float("-inf"))
    return nn.functional.cross_entropy(logits, tails)


def intra_cluster_loss(phase: torch.Tensor, cluster_ids: torch.Tensor) -> torch.Tensor:
    vec = unit_vectors_from_phase(phase)
    loss = phase.new_zeros(())
    count = 0
    for cluster in cluster_ids.unique():
        members = vec[cluster_ids == cluster]
        if members.size(0) < 2:
            continue
        centroid = nn.functional.normalize(members.mean(dim=0), dim=0)
        loss = loss + (1.0 - (members * centroid).sum(dim=-1)).mean()
        count += 1
    if count == 0:
        return phase.new_zeros(())
    return loss / count


def inter_cluster_loss(
    phase: torch.Tensor, cluster_ids: torch.Tensor, margin: float = 0.2
) -> torch.Tensor:
    vec = unit_vectors_from_phase(phase)
    prototypes = []
    for cluster in cluster_ids.unique():
        members = vec[cluster_ids == cluster]
        prototypes.append(nn.functional.normalize(members.mean(dim=0), dim=0))
    if len(prototypes) < 2:
        return phase.new_zeros(())
    proto = torch.stack(prototypes)
    sim = proto @ proto.T
    eye = torch.eye(sim.size(0), device=sim.device, dtype=torch.bool)
    off = sim.masked_select(~eye)
    return torch.relu(off - margin).mean()


def alignment_loss(model: ResidualRotate, llm_targets: torch.Tensor) -> torch.Tensor:
    """1 - cos(W_align(本体动词), LLM嵌入)。不反传进大模型。"""
    proj = model.projected_verb_vec()
    tgt = nn.functional.normalize(llm_targets, dim=-1)
    return (1.0 - (proj * tgt).sum(dim=-1)).mean()


def total_loss(
    model: ResidualRotate,
    heads: torch.Tensor,
    rels: torch.Tensor,
    tails: torch.Tensor,
    cluster_ids: torch.Tensor,
    true_tail_mask: torch.Tensor | None = None,
    llm_targets: torch.Tensor | None = None,
    w_task: float = 1.0,
    w_intra: float = 0.05,
    w_inter: float = 0.05,
    w_align: float = 0.0,
) -> tuple[torch.Tensor, dict[str, float]]:
    phase = model.relation_phase()
    l_task = task_loss(model, heads, rels, tails, true_tail_mask=true_tail_mask)
    l_intra = intra_cluster_loss(phase, cluster_ids)
    l_inter = inter_cluster_loss(phase, cluster_ids)
    if w_align > 0.0 and llm_targets is not None:
        l_align = alignment_loss(model, llm_targets)
    else:
        l_align = phase.new_zeros(())
    total = (
        w_task * l_task
        + w_intra * l_intra
        + w_inter * l_inter
        + w_align * l_align
    )
    return total, {
        "task": float(l_task.detach()),
        "intra": float(l_intra.detach()),
        "inter": float(l_inter.detach()),
        "align": float(l_align.detach()),
        "total": float(total.detach()),
    }
