# Evidence for the report

Each row points to actual output and the exchange that discusses it. The test program is
[review_checks.py](../analysis/review_checks.py) for command cases,
[route_checks.cpp](../analysis/route_checks.cpp) for the direct oracle, and the relevant
program in [bench/](../bench) for measurements. The retained changes can be inspected using
git show with the decision hashes below.

| Report fact | Consultation | Executed result | Decision commit |
|---|---|---|---|
| Ordered packed records | [Mira lines 42–50](../ai_logs/2026-11-24_chatgpt_team-mira.md#L42) | [layout-checks.txt, line 1](../results/layout-checks.txt#L1) | 2f0d036 |
| Packed quests after inserting a dependency | [Mira lines 54–66](../ai_logs/2026-11-24_chatgpt_team-mira.md#L54) | [quest-and-event-verification.txt, line 14](../results/quest-and-event-verification.txt#L14) | 2f0d036 |
| Rejected wrong weight, nonexistent bridge and reordered regions | [Jonah lines 63–73](../ai_logs/2026-11-24_chatgpt_team-jonah.md#L63) | [minimum-spanning-forest-verification.txt, line 14](../results/minimum-spanning-forest-verification.txt#L14) | df82425 |
| Different valid forest accepted, same-count cycle rejected | [Jonah lines 63–73](../ai_logs/2026-11-24_chatgpt_team-jonah.md#L63) | [minimum-spanning-forest-verification.txt, line 18](../results/minimum-spanning-forest-verification.txt#L18) | df82425 |
| Dense heap counts and matched scans | [Jonah lines 42–59](../ai_logs/2026-11-24_chatgpt_team-jonah.md#L42) | [prim_operations.csv, line 8](../results/prim_operations.csv#L8) | df82425 |
| First-walk ranking reverses when the second walk is counted | [Jonah lines 77–92](../ai_logs/2026-11-24_chatgpt_team-jonah.md#L77) | [rank on seed 2183, L39](../results/timings.csv#L39) and [both on seed 2183, L38](../results/timings.csv#L38) | df82425 |
| The two edge orders give distinct valid cycle witnesses | [Nia lines 36–46](../ai_logs/2026-11-24_chatgpt_team-nia.md#L36) | [shortest-path-and-cycle-verification.txt, line 3](../results/shortest-path-and-cycle-verification.txt#L3) | 15e55f2 |
| Fixed labels and equal scan counts | [Nia lines 64–74](../ai_logs/2026-11-24_chatgpt_team-nia.md#L64) | [control.csv, line 3](../results/control.csv#L3) | 15e55f2 |
| First-only quest predecessor mutation failed | [Leo lines 40–55](../ai_logs/2026-11-24_chatgpt_team-leo.md#L40) | [quest-and-event-verification.txt, line 17](../results/quest-and-event-verification.txt#L17) | 53cf4f8 |
| Alternate optimal source accepted by the checker | [Leo lines 59–76](../ai_logs/2026-11-24_chatgpt_team-leo.md#L59) | [quest-and-event-verification.txt, line 10](../results/quest-and-event-verification.txt#L10) | 53cf4f8 |
| Compatible controls and overlapping proportional ties | [Leo lines 80–97](../ai_logs/2026-11-24_chatgpt_team-leo.md#L80) | [event-cases.txt, line 4](../results/event-cases.txt#L4) | 53cf4f8 |

The source markers for packed map adjacency, best-key Prim, actual final-scan
relaxation and the first quest predecessor were checked in each draft-to-decision diff.
The integrated program preserves the behavior of master at 077adeb96e3bc823cf80fa3affd7db6d859f0b5a.
The log index shows the independently inspectable draft, decision and evidence commits.
