# Thing in itself, appearance, and abstraction

English version of [SKILL.md](SKILL.md). The rules are the same; change one, change the other. Terms follow the trilingual glossary at the top of [reference.md](reference.md).

## In one sentence

There is one real circuit board. CNC, XML and PCB (CT) are three independent representations of it, made by three different instruments, like three photos of one object from three camera positions. Representations do not map onto each other. Each representation leaf says only which leaf of the objective world it is abstracted from.

## Three layers, not two

Read strictly by the *Critique of Pure Reason* (*Kritik der reinen Vernunft*), what is loosely called the "thing in itself" (*Ding an sich*) splits into two layers. The layer that has geometry, layer counts and hole diameters is, for Kant, still an **object of experience** (*Gegenstand der Erfahrung*). It is not the thing in itself in the transcendental sense.

| Layer | Kant | Plato | In engineering | May it have leaves? |
|---|---|---|---|---|
| X transcendental object (*transzendentaler Gegenstand = X*) | thing in itself; noumenon in the negative sense (*Noumenon im negativen Verstande*); can be thought, not known; a limiting concept (*Grenzbegriff*) | No counterpart: Plato's Forms are knowable by reason; Kant's thing in itself is not | One empty anchor per kind of real thing, identity only | **No.** No DataProperty, no empirical relation |
| N objective world | object of experience, nature (*Natur*); the empirical "thing in itself" (the rain, relative to the rainbow) | The sensible particular, which participates (*methexis*) in a Form; its geometric frame is the *dianoia* of the divided line | The physical board: outline, stackup, copper, holes, backdrill segments, pads, traces, fiducials | Yes. Physical facts only |
| V representation (*Vorstellung*) | appearance (*Erscheinung*) given from one standpoint, through one instrument | *eikones*, images. *Republic* 598a: one bed seen from the side or the front only *appears* different | The CNC (manufacturing), XML (intermediate) and PCB (inspection) views | Yes. Private leaves allowed |

Plato's Form (*eidos*) corresponds to a **kind** in layer N (the Class itself), not to X. Do not treat the Form as the thing in itself.

## Three verb domains that never cross

| Domain | Verbs | Direction | Where |
|---|---|---|---|
| Empirical (being) | `part_of` `composes` `constitutes` `causes` `inheres_in`, RCC8 | Inside one layer | Inside N; inside each V |
| Representation | `abstracted_from` | V leaf → N leaf | Only across V→N |
| Limit | `appearance_of` | N kind → X | Thought only, never inference |

Rules:

1. **No edge between two views.** Not even for words that look the same. To compare, open a temporary comparison context: two view leaves are two appearances of one objective thing only when both are `abstracted_from` the same N leaf.
2. **N never points to V.** No `generates`, no `produces`. The objective thing does not generate files; an instrument represents it.
3. **No categories on X.** Kant allows categories only for objects of possible experience (A246/B303). X cannot `causes`, cannot be `part_of`, and has no quantity or quality. `appearance_of` says only that an appearance is always the appearance of something (Bxxvi–xxvii). Nothing may be inferred through it.
4. **`abstracted_from` is neither inheritance nor parthood.** It is not transitive, derives no subclass, and does not enter the ring hierarchy. Many-to-one is legal: a CNC hole and a PCB WIN may both be abstracted from the same physical hole.
5. **Private leaves stay unlinked.** A stop value, a tool number, an inspection threshold or a file section belongs to its view only. A leaf with no `abstracted_from` is private. Do not invent an objective counterpart for it.
6. **"Superset" holds only for what can be abstracted.** The private leaves of each view lie outside N, so N is not the literal union of the views.

## Building layer N after the Critique of Pure Reason

Layer N is not a free list of things. It is organised by the conditions of experience.

**Forms of intuition (*Formen der Anschauung*; transcendental aesthetic, *transzendentale Ästhetik*)**

- Space: every N leaf shares one board coordinate system, in millimetres. Region relations use RCC8. View coordinates are converted into this system before any comparison.
- Time: manufacturing is a sequence of events (lamination, drilling, plating, backdrilling). These are Perdurants, not properties of the board.

**Table of categories (*Tafel der Kategorien*, A80/B106) applied to leaves**

| Category | Engineering form | Example |
|---|---|---|
| Quantity: unity, plurality, totality | Extensive magnitude, count | One board; hole count; board width, height, thickness (mm) |
| Quality: reality, negation, limitation | Intensive magnitude, degree | Copper on the hole wall = reality; backdrill removes the plating = negation; stub length = limitation (the degree that remains) |
| Relation: substance and accident | What persists and its properties | The board and its copper layers persist; hole diameter is a property of the hole (`inheres_in`) |
| Relation: causality | An event causes a change; requires time order | Backdrilling `causes` the backdrill segment; plating `causes` the barrel copper |
| Relation: community | Mutual relation of things that exist at the same time | Copper layers coexist; RCC8 relation between a hole and a layer |
| Modality: possibility, actuality, necessity | **Not a leaf** | See below |

**Modality is not a property of the object.** Kant says modality adds nothing to the content of a concept; it states only the concept's relation to the faculty of cognition (A219/B266). "Allowed by the design" (possible), "measured so" (actual) and "necessary by law" (necessary) go on evidence, verdicts or identity status, never on a DataProperty of the board.

**Schematism (*Schematismus*, A137/B176): a category lands only with a measurable rule.** Quantity maps to millimetres and counts; quality to degree and threshold; causality to time order; community to an RCC8 test at one instant. If no measurable rule can be written for a category leaf, do not create the leaf yet.

**The three analogies of experience (*Analogien der Erfahrung*)**

1. Persistence of substance (first analogy): the board is one persisting thing through every process. Processes change its state; they do not change its UUID.
2. Causality (second analogy): `causes` connects events only. Things are never linked by `causes`.
3. Community (third analogy): parts that exist at the same time are linked by RCC8 or coexistence, never by causality.

## The thing-in-itself table (table of origins): a separate world file

The objective world (X and N) lives in its own file, called the thing-in-itself table or table of origins. Current instance: `OmronXmlGenerator/ODAAF/world/pcb-world.odaaf` (URI `urn:odaaf:world:pcb`, generated by `build_pcb_world.py` in the same folder).

**It has two purposes:**

1. **Reuse.** Any future PCB-related application can import it directly instead of rebuilding the board, holes, copper layers and backdrill segments. That is why it carries no Omron, BZOX, CNC, XML or PCB vocabulary and no file-format field.
2. **Return to the origin when lost.** When an `abstracted_from` mapping (formerly `projectsTo`) changes or is lost, or a leaf's counterpart in a peer view cannot be found, do not guess between views. Go back to the world file and find the origin leaf. Two view leaves correspond only if they abstract from the same world leaf (compared by UUID or URI).

**Layering: normally do not look outward.** While working inside CNC, XML or PCB, read only that domain's view file. Do not load or browse the world file; each sub-agent then needs fewer resources. Consult the thing-in-itself table only in three cases:

- extending the ontology with an objective leaf that other views may also show;
- converting or communicating across formats: CNC against XML, XML against PCB;
- the reverse compliance check (below).

**Referee and athletes are separate.** Rules (`Rule`, with evidence predicates and a violation template) are defined in the world file and attached to the world's structures. View files do not define these rules and do not write rules onto `abstracted_from` edges. Views are the athletes; the world is the referee. A check first follows `abstracted_from` from a view leaf to its world leaf, then applies the world's rules. Editing a view then cannot quietly change a rule, and changing a rule does not mean hunting through every view.

This mirrors the test in the *Critique of Practical Reason* (*Kritik der praktischen Vernunft*): a maxim (*Maxime*) is tested against the universal law (*Gesetz*), not the other way round. A view's practice is the maxim; the world's rule is the law. Only the structure is borrowed; the world's rules carry no moral meaning.

## How to split the files

- **The world file** holds only X and N, plus the referee rules. It is read-only: a project never writes it and never assigns identities to it.
- **A project ontology holds only views (V).** It imports the world with `<odaaf:Imports><odaaf:Import uri="urn:odaaf:world:pcb" href="../world/pcb-world.odaaf" /></odaaf:Imports>` and points at world leaves with `abstracted_from`. Save, MIB and published slices write only the file's own items; the MIB keeps a single `-- @odaaf.import` line.
- The workspace file `.odaafproj` lists both: the project views with `isPrimary="true"`, the world with `isPrimary="false"` (read-only reference).
- Ids must not clash across files: the world always uses the prefixes `cls-pw-`, `rel-pw-`, `str-pw-`, `rule-pw-`. On import a clashing id is skipped with a warning, never renamed.
- An OID need only be unique inside one `.odaaf` / `.mib`. Cross-file references use UUID or URI, never an OID prefix.
- When items move from a project to the world, the arcs the project used are recorded in `<odaaf:RetiredArcs>`; the allocator skips them and never reuses them.
- Later the views can be split further (one file each for CNC, XML, PCB and the index), so each domain's sub-agent reads only its own file.

## Checklist before and after a change

```
- [ ] Decide the layer of every new leaf: X / N / V
- [ ] No file format, software name or project name in an N leaf
- [ ] X has no DataProperty and no empirical relation
- [ ] No new direct V→V relation
- [ ] abstracted_from points only from V to N, and its object is a leaf, not a ring
- [ ] causes connects events only; things use part_of / inheres_in / RCC8
- [ ] Modality is on evidence or status, not a leaf
- [ ] No private view leaf has been forced into N
- [ ] New objective leaves go into the world file, not the project file
- [ ] New rules go into the world file; view files define no rules
- [ ] A missing peer counterpart was looked up through the world leaf, not patched with a direct view-to-view edge
- [ ] Moved items have their arcs recorded as retired
```

## Common mistakes

- Calling the physical board the "thing in itself" while giving it geometry and a layer count. Geometry belongs to the object of experience; the thing in itself, strictly, has no properties.
- Treating the three views as three universes and then building bridges between them.
- Expressing abstraction with inheritance: a CNC hole is not a subclass of the physical hole.
- Letting the objective world "generate" the views.
- Treating Plato's Form and Kant's thing in itself as the same thing. A Form is knowable by reason; the thing in itself is not.
- Writing rules into a view or onto an `abstracted_from` edge, so the athletes referee themselves.
- Loading the world file "just to look" while working inside one view. It is not needed unless you are extending the ontology, crossing formats or checking compliance.

## Sources

The trilingual glossary, page references, original meaning and limits of use are in [reference.md](reference.md).
