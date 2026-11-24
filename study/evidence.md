# Evidence for the report

Each row gives the AI exchange and a recorded program output or measurement. Command checks are in [review_checks.py](../analysis/review_checks.py). The [direct route oracle](../analysis/route_checks.cpp) and [bench programs](../bench/) produce the remaining data.

| Finding | Consultation | Recorded result | Review commit |
|---|---|---|---|
| Valid input excludes duplicate bridges and self-loops | [Mira exchange 2, L143](../ai_logs/2026-11-24_chatgpt_context-mira.md#L143) | [map-adjacency-verification.txt, L18](../results/map-adjacency-verification.txt#L18) | `5b39c56` |
| Changed bridge ID leaves labels unchanged | [Mira exchange 3, L323](../ai_logs/2026-11-24_chatgpt_context-mira.md#L323) | [layout-checks.txt, L2](../results/layout-checks.txt#L2) | `5b39c56` |
| Hub control isolates the layout phases | [Mira exchange 3, L349](../ai_logs/2026-11-24_chatgpt_context-mira.md#L349) | [Packed star epochs for seed 2183, L520](../results/layout_epochs.csv#L520) | `5b39c56` |
| Owned array capacity excludes temporary buffers | [Mira exchange 5, L581](../ai_logs/2026-11-24_chatgpt_context-mira.md#L581) | [adjacency-capacity.csv, L2](../results/adjacency-capacity.csv#L2) | `5b39c56` |
| The printer groups islands in ascending ID order | [Mira exchange 7, L794](../ai_logs/2026-11-24_chatgpt_context-mira.md#L794) | [map-adjacency-verification.txt, L3](../results/map-adjacency-verification.txt#L3) | `5b39c56` |
| Feasible forest of cost 12 rejected | [Jonah exchange 2, L153](../ai_logs/2026-11-24_chatgpt_context-jonah.md#L153) | [minimum-spanning-forest-verification.txt, L28](../results/minimum-spanning-forest-verification.txt#L28) | `50a3bb2` |
| Dense best-key cuts heap work at equal scans | [Jonah exchange 3, L296](../ai_logs/2026-11-24_chatgpt_context-jonah.md#L296) | [prim_operations.csv, L8](../results/prim_operations.csv#L8) | `50a3bb2` |
| Depth-six cold and repeated finds count both traversals | [Jonah exchange 5, L592](../ai_logs/2026-11-24_chatgpt_context-jonah.md#L592) | [union-find-revisits.csv, L9](../results/union-find-revisits.csv#L9) | `50a3bb2` |
| Prim and Kruskal compared at 4,096 islands | [Jonah exchange 7, L884](../ai_logs/2026-11-24_chatgpt_context-jonah.md#L884) | [Prim on seed 2183, L17](../results/timings.csv#L17) and [Kruskal on seed 2183, L16](../results/timings.csv#L16) | `50a3bb2` |
| First scan-V update can target a vertex beyond the cycle | [Nia exchange 3, L141](../ai_logs/2026-11-24_chatgpt_context-nia.md#L141) | [shortest-path-and-cycle-verification.txt, L9](../results/shortest-path-and-cycle-verification.txt#L9) | `ba75325` |
| Direct oracle and separate directed witnesses | [Nia exchange 4, L225](../ai_logs/2026-11-24_chatgpt_context-nia.md#L225) | [route-oracle.txt, L2](../results/route-oracle.txt#L2) | `ba75325` |
| The reachable zero-total cycle has a finite answer | [Nia exchange 4, L225](../ai_logs/2026-11-24_chatgpt_context-nia.md#L225) | [shortest-path-and-cycle-verification.txt, L14](../results/shortest-path-and-cycle-verification.txt#L14) | `ba75325` |
| Replay at final labels preserves the ordering in elapsed time | [Nia exchange 7, L465](../ai_logs/2026-11-24_chatgpt_context-nia.md#L465) | [control.csv, L4](../results/control.csv#L4) | `ba75325` |
| Keeping only the first quest predecessor drops a later optimum | [Leo exchange 3, L210](../ai_logs/2026-11-24_chatgpt_context-leo.md#L210) | [quest-and-event-verification.txt, L26](../results/quest-and-event-verification.txt#L26) | `5098fb4` |
| The actual positive tie was repeatable and the other optimum was accepted | [Leo exchange 4, L293](../ai_logs/2026-11-24_chatgpt_context-leo.md#L293) | [quest-and-event-verification.txt, L13](../results/quest-and-event-verification.txt#L13) | `5098fb4` |
| Equal finish and touching intervals give optimal reward 14 | [Leo exchange 6, L528](../ai_logs/2026-11-24_chatgpt_context-leo.md#L528) | [quest-and-event-verification.txt, L25](../results/quest-and-event-verification.txt#L25) | `5098fb4` |
| Overlapping order reversal and fixed-order bonus have different effects | [Leo exchange 7, L626](../ai_logs/2026-11-24_chatgpt_context-leo.md#L626) | [event-cases.txt, L4](../results/event-cases.txt#L4) | `5098fb4` |

## Chronology

The prior study assembled the implementation before these four consultations. The review commits record the results of examining saved intermediate drafts and checking individual claims against the completed program. [Chronology](chronology.json) records the draft and implementation commits, conversation times and new review commits.

| Role | Draft | Earlier implementation decision | Review |
|---|---|---|---|
| Mira | `119a7a8` | `2f0d036` | `5b39c56` |
| Jonah | `37c7ed3` | `df82425` | `50a3bb2` |
| Nia | `db40acf` | `15e55f2` | `ba75325` |
| Leo | `2ed3290` | `53cf4f8` | `5098fb4` |

The integrated program preserves the behavior of master at `077adeb96e3bc823cf80fa3affd7db6d859f0b5a`.
