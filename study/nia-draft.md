# Nia's route draft

After V−1 Bellman–Ford scans, every simple source path has had enough scans to contribute its final label. The current extra scan applies all strict relaxations, remembers the last destination and follows V predecessors to a cycle. A predecessor points against traversal, so reverse the discovered cycle to get directed arcs. I want to distinguish detecting a reachable strict improvement from preserving its witness.

For the required Dijkstra example I use s→a (2), s→b (4), b→a (−3), a→t (1). Extract-once processes a and t before b, so improving a later does not rescan a→t. Actual shortest labels are 0,1,4,2 and the stale target label in the extract-once trace is 3. At most three vertices give simple paths of at most two edges, so relaxing into settled labels would already set correct final distances. Freezing their labels permits a three-vertex failure. With one negative edge the tail has its final label when first expanded, since using the edge to return to its tail adds a nonnegative cycle. Keys are nondecreasing before and after that one negative relaxation, and each vertex expands at most once per phase.

For nonnegative benchmark inputs I expect the update-free scan to follow the last successful one. Minimum shortest-path hop depth bounds the number of successful scans, and edge order can let one in-place scan propagate several hops.
