# Verbs per aspect

`existing` = already in `data/project_dna/verbs.json`. `proposed` = needs approval before use in
training; until then it may appear in drafts only.

| Aspect | Verb | Status | Direction (stored) | Meaning |
|---|---|---|---|---|
| whatOf | `composes` | existing | whole → part | same-kind elements stacked |
| whatOf | `constitutes` | existing | matter → object | different kinds make one object |
| whatOf | `part_of` | existing | part → whole | mereology |
| how | `constructs_from` | existing (schema_only) | result or process → material | build from limited given information |
| how | `selects` | proposed | criteria → chosen candidate | discovery among existing candidates |
| how | `constrains` | existing | rule → process or result | must stay within |
| how | `forbids` | existing (schema_only) | rule or region → result | must not happen |
| how | `obliges` | existing (schema_only) | rule → process | must happen |
| what | `EQ` | existing (RCC8) | candidate → role object | same region, two roles |
| where | `located_in` | proposed | object → frame | membership in a coordinate frame or container |
| where | RCC8 (`DC`, `EC`, `PO`, `EQ`, `TPP`, `NTPP`, inverses) | existing | either, as declared | qualitative spatial relation in one frame |
| where | `pass_through` | existing (extension) | object → region | crosses, not an RCC8 base relation |
| who | `participates_in` | proposed | agent or tool → event | takes part, is not the whole cause |
| when | `causes` | existing | event → direct product | efficient cause inside a group |
| when | `precedes` | existing | earlier step → later step | time order inside one procedure; peer, never a layer |
| why | `aims_at` | existing (schema_only) | role → its end inside the group | purposiveness |
| why.effect | `leads_to` | proposed | result → effect, function or entity | outward effect of the group |
| why.use | `serves` | proposed | result → business or capability | what it is used for |
| why.consequence | `results_in` | proposed | group result → consequence | what follows, wanted or not |
| — | `abstracted_from` | existing | view leaf → world leaf | representation; never a cause role |

## Direction rules

- Store one direction per fact. Inverse readings (`caused_by`, `part_has`, `follows`) are views.
- `causes` vs `results_in`: `causes` is an event producing its direct product inside a group;
  `results_in` is the downstream outcome of a group's result, usually in the next layer.
- `aims_at` vs `leads_to`: `aims_at` is a purpose of a role inside the group; `leads_to` is the
  effect of the whole group outside it.
- `located_in` vs `part_of` vs `TPP/NTPP`: frame membership vs mereology vs region containment.

## Knowledge-layer direction

When added to the closed table in `RelationRoles.Feeds` (ODAAF), proposed verbs feed as:

| Verb | Input | Output |
|---|---|---|
| `selects` | criteria | chosen candidate |
| `precedes` | none (peer) | — |
| `located_in` | object | frame (like `part_of`: the frame is the larger context) |
| `leads_to`, `serves`, `results_in` | source | target |

`precedes` is approved for the order of steps inside one procedure. It stays a peer: it is not added to `RelationRoles.Feeds`, and it is not drawn on the experience graph. RCC8 stays a peer too. Neither sets a layer.
