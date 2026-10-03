# Patterns

## 1. Aspects

Each role leaf in a group has a **cause** (Aristotle) and an **aspect** (5W1H). The aspect says
which question the leaf answers.

| Aspect | Question | Cause | What fills it |
|---|---|---|---|
| `whatOf` | What is it made of? Which components? | Material | parts, ingredients, candidates, input facts |
| `how` | How is it built or found? | Formal | method, rule, algorithm, criteria, constraints |
| `what` | What do we get? | Formal | the resulting object (its form) |
| `where` | Where? In what frame? | Formal (spatial form) | frame (board, layer, coordinate system), RCC8 neighbours |
| `who` | Who or what tool does it? | Efficient | agent, tool, software, machine |
| `when` | When? Before or after what? | Efficient (time) | event, step, ordering |
| `why` | What for? With what result? | Final | effect, function, business use, consequence |

Notes:

- `what` is formal, not material: the result is the realized form ("what it is").
- `where` is spatial form. Coordinates are `DataProperty` leaves (mm, layer index), not edges.
  Edges carry only the frame (`located_in`) and qualitative relations (RCC8).
- `why` has three sub-aspects, kept apart because they lift differently:
  - `effect` – what effect, function or entity the result produces (`leads_to`);
  - `use` – which business it serves (`serves`);
  - `consequence` – what follows, wanted or not, including failure modes (`results_in`).

## 2. Patterns

Every pattern decomposes into binary triples through the reified group. P0 is the atom.

| Id | Name | Question it answers | Required aspects | Optional |
|---|---|---|---|---|
| P0 | Triple | single fact | — | — |
| P1 | Composition | what is it made of | whatOf (≥1), what (1) | where |
| P2 | Construction | how is a new thing built | whatOf (≥1), how (≥1), what (1) | who, when, where |
| P3 | Discovery | how is an existing thing found | whatOf (candidates ≥1), how (criteria ≥1), what (1, one of the candidates) | where |
| P4 | Process | who does it and when | who or when (≥1), what (1) | whatOf |
| P5 | Placement | where is it | what (1), where (≥1) | — |
| P6 | Guard | what is allowed or forbidden | how (rule ≥1), what (the guarded group or result) | why |
| P7 | Purpose | what for | what (1), why (≥1) | — |
| P8 | Full group | all of it | whatOf, how, what, who or when, why | where |
| P9 | Chain (lift) | how one layer feeds the next | two groups; a `why`/`what` of the first is `whatOf` of the second | — |

### P0 Triple

`subject verb object`, binary, one verb from `verbs.md`. Existing triples stay valid; a group only
references them.

### P1 Composition

Parts → whole. No method needed because the whole is just the parts taken together.

- `composes`: same kind stacked (an array of pads).
- `constitutes`: different kinds make one object (copper + barrel + segment make a hole).
- Do not use for peers of the same kind (`subordinate_to`).

### P2 Construction

Material plus method → a result that did not exist before the method ran.

```text
whatOf: M1..Mn   how: rule / algorithm   what: R
R constructs_from Mi        rule constrains (process that makes R)
```

Distinguish from P1: if removing the method leaves no R, it is construction.

### P3 Discovery

Candidates plus criteria → one candidate selected. The result's identity is one of the inputs.

```text
whatOf: candidates C   how: criteria K   what: c in C
K selects c   (proposed)      c EQ role-object   (when the role has its own class)
```

Distinguish from P2: nothing new is made; something already there is identified.

### P4 Process

Agent and event. `participates_in` links a persisting thing to an event; `causes` links an
event to its direct product; `precedes` orders events.

- `causes` stays inside a group (event → direct product).
- The group's downstream outcome is `results_in` (P7), not `causes`.

### P5 Placement

`what located_in frame` plus RCC8 to neighbours. A frame is a coordinate system or a container
region (board, layer, panel). Containment of regions inside the same frame uses RCC8
(`TPP`, `NTPP`); membership in a frame uses `located_in`; mereology uses `part_of`.

### P6 Guard

`constrains`, `forbids`, `obliges`. A guard targets a process or a result, never a person's
permission. Guards are evaluated deterministically by the referee; AI may propose guards only.

### P7 Purpose

```text
what leads_to E      (effect / function / produced entity)
what serves B        (business use)
what results_in Q    (consequence; include known failure modes)
```

`aims_at` is the judgment-level purpose of a role inside a group (for example a pad pair aims at
its traces). `leads_to` is the outward effect of the whole group.

### P8 Full group

P1 or P2 or P3 (material and form) + P4 (efficient) + P7 (final), optionally P5 and P6.
An ODAAF lift `Structure` (`isLift="true"`) already requires all four causes; a full group is a
lift when its `why` lands in another layer.

### P9 Chain

The output or effect of one group is the material of the next:

```text
G1.what = R      G2.whatOf includes R
G1.why.effect = E   G2.whatOf includes E
```

Chains are how the system climbs layers. Each step keeps its own group and OID; a chain is never
flattened into one group.

## 3. Completeness rules (referee candidates)

1. A group has exactly one `what`.
2. P2 and P3 need at least one `how`; P3's `what` must also appear in `whatOf`.
3. P4 needs `who` or `when`; an event in `when` must be a `Perdurant`.
4. P8 needs all four causes; missing ones are listed as gaps, not guessed.
5. `why.consequence` with a failure mode must name the guard (P6) that prevents it, or be marked
   `unguarded` for human review.
6. No role leaf may point at an X-layer class; `abstracted_from` never fills a role.

## 4. Layer heuristic

Relates to `docs/system-requirements/02-knowledge-layers.md`.

- `whatOf` has several distinct elements → the group sits in the **middle** of a major layer
  (connection), not near the bottom.
- `whatOf` is a single particle of the focus → near the **bottom** (just above the boundary).
- `why.use` names a business and nothing in the focus consumes `what` → **top** (structure).
- The minimal grain of a focus is the set of `whatOf` leaves that no group in the focus produces.

## 5. Encoding in ODAAF today

| Group part | ODAAF element |
|---|---|
| group | `Structure` (own OID in branch 6) |
| role leaf | `CauseRole` leaf (`role` = Material / Formal / Efficient / Final, `targetKind`, `targetId`, `ordinal`) |
| aspect | **gap**: no attribute yet. Until ODAAF adds `aspect`, write it as a label prefix: `[how] Z rule` |
| facts | existing `ObjectProperty` edges listed in `RelationRefs` |
| guards | `Rule` listed in `RuleRefs`, bound by `RuleBinding` |
| layer move | `isLift`, `sourceLayer`, `targetLayer` |

Proposed ODAAF change: add `aspect` (`whatOf|how|what|where|who|when|why`) and `whySub`
(`effect|use|consequence`) to `CauseRole`, with a referee check for the rules in section 3.
