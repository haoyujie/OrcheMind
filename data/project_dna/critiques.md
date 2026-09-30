# Three Critiques: schema first, text later

The three critiques enter OrcheMind as verbs in `verbs.json`, not as a downloaded book used for training.

| Critique | Job in this system | Verbs | Cluster |
| --- | --- | --- | --- |
| Pure Reason | Construct under limited information | `constructs_from` | inherence |
| Practical Reason | What must not be done, and what ought to be done | `forbids`, `obliges` | norm |
| Judgment | What is taken as the end (purposiveness) | `aims_at` | cause |

Judgment is not the ethics book. Ethics and “must not” sit in Practical Reason. Judgment says what a construction is for.

These four verbs are `schema_only`. OA `v001` does not use them. `v001` stays frozen. A later `v002` may use them after hand-labeled triples exist.

## Construction verbs (do not collapse these)

| code | name | Means |
| --- | --- | --- |
| `constitutes` | constitutes | A different kind constitutes the object |
| `composes` | composes | Same elements stacked, like an array |
| `part_of` | is part of | X is part of Y |
| `subordinate_to` | is subordinate to | Peer of the same kind is affiliated under another peer |

OA `v001` still uses `constitutes` for invoice–expense. That wording is coarser than this split. Correct it in `v002`, do not edit `v001`.

## If a project has too few examples

Do not download Kant and train on the prose.

1. Keep the verb list closed.
2. Write 10 to 20 abstract triples per new verb, same shape as `data/triples/rcc8_abstract.jsonl`: head, relation, tail. Relation is the English `code`.
3. Hold those triples out of any weight update until a second set exists for checking.
4. Use a real project only where the verb already has a fact: OA policies can later illustrate `forbids` / `obliges`. Pure reason and judgment still need the hand-written triples first.

## Public-domain text, for reading and labeling only

Put nothing from these sites into `data/triples/` automatically.

- Critique of Pure Reason, Meiklejohn translation: Project Gutenberg, ebook 4280.
- Critique of Practical Reason, Abbott translation: Project Gutenberg, ebook 5683.
- Critique of Judgment, Bernard translation: Project Gutenberg, ebook 48433.
- German source for checking a sentence: Korpora Kant (Universität Duisburg-Essen).

The Cambridge editions are copyrighted. Do not copy them into this repo.
