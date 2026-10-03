# Example: Z extraction (BZOX-862)

Source: `g:\myfuture\OntoLib\BzoxOdaaf\bzox\zextract-rules.odaaf` (module arc 862). Every edge id
below exists in that file unless marked `proposed` or `gap`.

Z extraction is not one fact. It is a chain of three groups:

```text
G1 Discovery: which pad hosts the Z window
      └─ what = host land ──► material of G2
G2 Full group: build the Z extraction window
      └─ why.effect = PCB WIN 296 ──► material of G3
G3 (sketch): inspection that consumes the window
```

## G1 Discovery (P3): find the host land

| Aspect | Cause | Target | Edge |
|---|---|---|---|
| whatOf (candidates) | Material | `cls-z-pad` on the signal layer | `rel-backdrill-constitutes` (backdrill land constitutes pad) |
| how (criteria) | Formal | `cls-z-backdrill`, `cls-z-pad-pair` | `rel-pad-in-pair` (pad subordinate_to pair), `rel-pair-aims-traces` |
| what | Formal | `cls-z-host-land` | `rel-pad-as-host` (pad EQ host land) |
| where | Formal | `cls-z-signal-layer` | `rel-layer-hosts-backdrill` |

Nothing new is made; one existing pad is identified, so this is discovery, not construction.

## G2 Full group (P8): build the Z extraction window

| Aspect | Cause | Target | Edge |
|---|---|---|---|
| whatOf | Material | `cls-z-signal-layer` | `rel-place-from-layer` (placement constructs_from layer) |
| whatOf | Material | `cls-z-host-land` (from G1) | `rel-window-has-land` (window composes host land) |
| whatOf | Material | `cls-z-trace`, `cls-z-pad` | `rel-trace-has-z`, `rel-pad-associates-z` |
| whatOf | Material | `cls-z-tool-binding` | `rel-binding-aims-layer` |
| how | Formal | `cls-z-rule` with `cls-z-param` | `rel-rule-has-param`, `rel-rule-constrains-place` |
| how (guard, P6) | Formal | `cls-z-keepout` | `rel-keepout-forbids-z` (keepout forbids window) |
| what | Formal | `cls-z-window` | — (the one result) |
| where | Formal | `cls-z-signal-layer` | `rel-window-on-layer` (window part_of layer) |
| where | Formal | coordinates | DataProperty leaves of the window (mm, layer index) |
| when | Efficient | `cls-z-placement` | `rel-place-causes-window` (placement causes window) |
| when | Efficient | after backdrill land is known, before XML→PCB export | `precedes` (proposed) |
| who | Efficient | ZExtractStudio / BHEditor | gap: no tool class yet; `participates_in` (proposed) |
| why.effect | Final | `cls-z-pcb-win` (PCB WIN 296) | `rel-window-constitutes-win`; reading as `leads_to` is proposed |
| why.use | Final | AOI backdrill stub inspection (BD8161 / BD8164 criteria) | `serves` (proposed); target class lives in the Omron view |
| why.consequence | Final | wrong placement → false NG or missed stub | `results_in` (proposed); guarded by `rel-keepout-forbids-z` and the rule params |

Layer reading: `whatOf` has several distinct elements, so G2 sits in the middle of the PCB
manufacturing layer (connection), not at its bottom. Its particles (the minimal grain of this
focus) are the signal layer, pads, traces and backdrill lands: nothing in the focus produces them.

Open choice for a person: `rel-window-on-layer` is stored as `part_of`. If the layer is meant as a
frame rather than a whole, it becomes `located_in` (proposed). The arc stays; only the verb and its
documented meaning change, with a version note.

## G3 Chain (P9, sketch)

```text
G3.whatOf = PCB WIN 296 (G2.why.effect) + criterion item 8161 / 8164
G3.how    = inspection criteria
G3.what   = inspection result per hole
G3.why    = serves backdrill quality acceptance; results_in pass / NG
```

## World anchors

Positions of these view leaves come from the PCB world (`docs/world-anchored-training.md`).
In `omron-nodes.odaaf`, `cls-bzoxClPad` and `cls-Land` are abstracted from `cls-pw-pad`
(`rel-xml-pad-abstracts-pad`, `rel-xml-land-projects-pad`), and `cls-bzoxClSignalTrace` from
`cls-pw-trace` (`rel-xml-trace-abstracts-trace`). Only the CNC backdrill is anchored to
`cls-pw-backdrill-segment` (`rel-cnc-backdrill-projects-cavity`). The Z-module classes
(`cls-z-*`) have no `abstracted_from` edges yet, so they are private leaves: they take the
nearest anchored neighbour and are flagged as candidates for missing anchor edges. The group itself has an OID in module 862 and does not move
when the vectors are retrained.

## As JSON

```json
{
  "id": "grp-z-window",
  "pattern": "P8",
  "module": "urn:odaaf:bzox:zextract",
  "roles": [
    {"cause": "Material", "aspect": "whatOf", "target": "cls-z-signal-layer", "edge": "rel-place-from-layer"},
    {"cause": "Material", "aspect": "whatOf", "target": "cls-z-host-land", "edge": "rel-window-has-land", "from": "grp-z-host"},
    {"cause": "Formal", "aspect": "how", "target": "cls-z-rule", "edge": "rel-rule-constrains-place"},
    {"cause": "Formal", "aspect": "how", "target": "cls-z-keepout", "edge": "rel-keepout-forbids-z", "guard": true},
    {"cause": "Formal", "aspect": "what", "target": "cls-z-window"},
    {"cause": "Formal", "aspect": "where", "target": "cls-z-signal-layer", "edge": "rel-window-on-layer"},
    {"cause": "Efficient", "aspect": "when", "target": "cls-z-placement", "edge": "rel-place-causes-window"},
    {"cause": "Final", "aspect": "why", "whySub": "effect", "target": "cls-z-pcb-win", "edge": "rel-window-constitutes-win"}
  ],
  "gaps": ["who: no tool class", "why.use: serves (proposed)", "why.consequence: results_in (proposed)"]
}
```
