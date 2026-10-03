---
name: orchemind-world-anchored-training
description: >-
  Strategy for training and using OrcheMind positions: train only the world (thing-in-itself,
  N layer) ontology, which is the superset; derive every view (CNC / XML / PCB / index) position
  from the world leaf it is abstracted_from. Use when planning OrcheMind training, embeddings,
  harmonic coordinates, Q placement, coordinate storage, cross-view translation, or when the user
  mentions 训练 OrcheMind、物自体、超集、锚定、位置向量、世界坐标、视图位置.
---

# World-anchored training

Authoritative note (Chinese): `docs/world-anchored-training.md`.
Related: `docs/system-requirements/03-spaces.md`, `05-coordinates.md`,
skill `noumenon-phenomenon-projection`, rules `rules/info-group-patterns/`.

## Core rule

Train positions on the **world N layer only**. Every view leaf gets its position from its anchor:

```text
pos(view_leaf, v) = pos(world_anchor, v) + eps * offset(view_module_arc, view_leaf_oid)
```

- `world_anchor` = target of the view leaf's `abstracted_from` edge.
- `offset` is a hash of the canonical OID (harmonic scheme 2): deterministic, untrained, recomputable.
- Several anchors: weighted centre; equal weights unless the edge says otherwise.

## Layer handling

| Layer | Position | Source |
|---|---|---|
| X (transcendental object) | none | identity only, never trained |
| N (world, empirical nature) | trained | world-internal edges only |
| V (views) | derived | anchor + small offset |

## Checklist before training

1. Load the world file (e.g. `g:\myfuture\OntoLib\BzoxOdaaf\world\pcb-world.odaaf`) and its importing views.
2. Referee findings must be 0 (`RefereeCheck`): no view-to-view edges, every `abstracted_from` targets the N layer.
3. Training set = N-layer classes and N-internal edges. Do not include view classes or `abstracted_from` edges as training triples.
4. Sample weight = endorsement: number of distinct views abstracting the world leaf (via hubs weigh more).
5. Key every vector by canonical OID (`privateRoot.2.moduleArc.branch.arc`), never by id or UUID.
6. Write `data/world_coords/<world>/<version>.json`; do not store view vectors.

## Private (unanchored) view leaves

- Position = nearest anchored neighbour inside the same view + offset; flag `anchored: false`.
- Each one is a candidate for a missing world leaf. AI proposes; a person decides; the world file is edited in ODAAF.

## Versioning

- New world version → retrain world vectors; recompute all view vectors by formula.
- Align versions by OID; large displacement = mutation, list for human review.
- World split/merge → migration map old OID → new OID(s); anchors follow; old OID retired.

## Using positions

```text
Q → its view or world leaf → world anchor → neighbours / paths / rules in world space
  → back to the target view through abstracted_from
```

Cross-view translation is a lookup through the shared world OID, not a learned view-to-view map.

## Never

- Train a view-to-view mapping or bring back `corresponds` edges.
- Give X a coordinate.
- Use a vector as a primary key.
- Let a learned position authorize an action; positions retrieve and propose only.

## Training corpus shape

Group triples into **information groups** (four causes + 5W1H) as defined in
`rules/info-group-patterns/`. One group = one reified context with typed role edges; groups chain
when the final cause of one becomes the material of the next.
