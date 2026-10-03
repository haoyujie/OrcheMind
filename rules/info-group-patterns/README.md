# Information-group patterns

Extends the binary triple grammar (`subject verb object`) into **information groups**:
one reified context that answers the four causes and 5W1H together.

```text
Material (what of)  --how-->  Formal result (what)        <- the four causes
   by Efficient (who, when) in a frame (where)
   for a Final purpose (why): leads_to / serves / results_in   <- lift to the next layer
```

Status: **draft**. Verbs marked `proposed` are not in `data/project_dna/verbs.json` yet and are
not training targets until a person approves them and hand-labelled groups exist.

## Files

| File | Content |
|---|---|
| [patterns.md](patterns.md) | Aspects, the ten patterns (P0 to P9), completeness rules, layer heuristic, encoding in ODAAF |
| [verbs.md](verbs.md) | Verb table per aspect; existing vs proposed; direction rules |
| [examples/z-extract.md](examples/z-extract.md) | Z-extract (BZOX-862) written as one full information group plus its chain |
| [info-group.schema.json](info-group.schema.json) | JSON shape for a group, for training corpora and exports |

## Five rules that hold everywhere

1. **Edges stay binary.** A group is a reified context (an ODAAF `Structure` with `CauseRole`
   leaves), never one n-ary edge.
2. **Causes are roles, not kinds.** The same class can be material in one group and the result
   in another. The output of group *n* may be the material of group *n+1*.
3. **Formal before efficient.** A process (who/when) is authorized only after its how
   (method, constraints) and its what (result form) are declared.
4. **Store one direction.** `A results_in B` is stored; "B is caused by A" is its inverse reading
   and is never stored as a second edge.
5. **Representation is not causation.** `abstracted_from` anchors a view leaf to a world leaf.
   It never fills a cause role. Groups are written over world leaves where possible, so their
   positions are stable (see `docs/world-anchored-training.md`).

Related: OrcheEditor `rules/four-causes-transformer-rules.md`, `rules/ontology-modeling-rules.md`.
