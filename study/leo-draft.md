# Leo's scheduling draft

The quest recurrence is F(v)=duration(v)+max({F(u) for direct prerequisites u} ∪ {0}). Readiness is decided by indegree, so every contribution arrives before processing v. An unfinished topological pass witnesses a cycle anywhere. My draft stores a predecessor on a strict improvement only. For durations 3,2,4 along 0→1→2, finish 9 and chain 0,1,2 should both be printed.

For weighted events sorted by finish, the earlier compatible events form a prefix. With p(j) its length, W(0)=0 and W(j)=max(W(j−1), reward(j)+W(p(j))). Trace back by the chosen arc to j−1 or p(j). Use binary search for the prefix, so time is O(n log n). For unweighted attendance, replace an optimal continuation's first event with the earliest finisher. The new finish leaves every later compatible start feasible and preserves the number selected. Repeat on the feasible suffix.

Before consultation I chose these counterexamples. Largest reward selects [0,3) worth 9 instead of two touching intervals worth 6 and 4. Earliest finish selects [0,1) worth 1 instead of [0,2) worth 2. Density selects [0,1) worth 3 instead of [0,2) worth 5. Two events cannot defeat largest reward because they either fit together or the greater reward wins. One event cannot defeat either other rule.
