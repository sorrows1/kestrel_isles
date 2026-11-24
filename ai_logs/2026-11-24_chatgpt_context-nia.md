https://chatgpt.com/share/6b0556b1-4910-83ec-ab54-3d04027b0180

Browser consultation for Nia, a fictional role operated by Codex.
Post-project review using the saved draft at db40acf.
The share page also shows the original rendered formatting.

## Prompt 1

I'm Nia, handling shortest paths in a standalone review of Kestrel Isles. The driver has two networks. fastest uses undirected bridges with nonnegative lengths and should return a minimum-cost bridge walk using binary-heap Dijkstra. stamina uses directed sea routes with signed costs and calls Bellman–Ford. On stamina s t, any negative cycle reachable from s takes precedence over the voyage to t, even if t is unreachable. The printed exploit must follow directed arcs and repeat the start at the end. Islands outside the reachable part should stay at an infinity sentinel, and route costs are stored in signed 64-bit values.

My Bellman–Ford draft initializes only s to zero, then scans the edge list in input order. On each strict improvement it saves the distance and parent together. A quiet scan returns distances. If scan V is still active, I remember the last updated vertex, walk V parent links, then reverse the collected vertices because links point toward predecessors. The draft applies all improving edges on scan V. I think V−1 scans already cover the best simple source paths, while the final scan serves both a detection and a witness role.

I want to decide whether doing all the final-scan work is necessary without damaging the witness. My first check will use a small reachable negative cycle and trace the parents before the final scan. I will work out the correctness argument and run cases locally. What assumption about the starting vertex of the predecessor walk should I challenge?

## Response 1

The assumption to challenge is that **the last updated vertex from scan&#x20;**\\(V\\)**&#x20;is necessarily the correct starting point for the predecessor walk without first establishing that the parent chain is valid and leads into a cycle.**

There are two separate questions here:

1. Does the vertex updated on scan \\(V\\) have a predecessor chain that eventually enters a negative cycle?
2. Does walking exactly \\(V\\) parent links from that vertex guarantee that you are inside the cycle?

For the second question, the important distinction is that the parent graph contains at most one predecessor per reachable vertex, and the graph has only \\(V\\) vertices. If the predecessor chain is valid, following \\(V\\) links forces a repeated vertex. The repeated portion gives a cycle.

**Your final-scan strategy is important here.**

- After \\(V-1\\) complete scans, every shortest simple path has had enough relaxation opportunities.
- A strict improvement during scan \\(V\\) establishes that a reachable negative cycle exists.
- Recording the last updated vertex and its parent information allows witness extraction.
- Applying all improvements during scan \\(V\\) is not inherently required just to detect a negative cycle, but changing this behavior can affect whether the saved predecessor chain reliably identifies one.

One subtlety worth investigating in your first experiment: **the last updated vertex is not necessarily itself on the negative cycle.** It may be downstream from it.

That's precisely why the \\(V\\)-link predecessor walk matters: it moves past any downstream portion before you begin collecting the cycle.

For your local test, compare:

- Starting the predecessor walk from the last updated vertex.
- Starting it from an earlier vertex updated during the final scan.
- Stopping the final scan at its first strict improvement versus completing it.

Check whether each resulting witness is a closed, directed cycle and whether its total cost is negative.

Your key correctness question is whether the predecessor chain you use corresponds to the distance updates that established the final-scan improvement. That is more important than simply proving that a vertex was updated.

## Prompt 2

I traced two actual command cases. For ordered arcs (0,2,-1),(1,0,1),(1,2,-3),(2,1,1), after two complete scans I got [0,-2,-3]. The first improving arc is 1→0, while 0 still has no parent. The full-scan draft nevertheless prints a legal negative cycle because it applies updates. For a downstream case I tried (2,3,0),(0,1,0),(1,2,-3),(2,1,1). The first possible update on scan 4 is to sink 3, and the draft prints exploit 2 1 2. The witness starts inside the cycle even though the first improving target was outside it.

My argument for an early stop is that a real update gives the new target a label below every simple source path. For any saved edge p→v, d[v] >= d[p]+w because p can only decrease after v was assigned. A parent walk ending at the source with no parent contradicts that path bound. On a cycle the earliest last assignment has a parent assigned later, giving a strict inequality. I want to share one relaxation routine between the ordinary scans and an explicit check scan that stops only after applying an improvement. Can you challenge the proof's assumptions, especially its claim of strictly negative weight?

## Response 2

Your two command cases expose an important distinction: the first improving target on scan \\(V\\) is not necessarily on the cycle, but it can still be a valid starting point for predecessor extraction.

Your proposed early-stop approach is worth investigating. However, I would challenge three assumptions in the proof, particularly the strict-negativity argument.

### **1. The saved-parent inequality**

You claim that for every saved parent edge \\(p \to v\\),

\\[ d[v] \ge d[p]+w(p,v). \\]

The reasoning is that \\(p\\)'s distance can only decrease after \\(v\\) receives its assignment.

This needs careful qualification:

- The inequality holds immediately after assigning \\(d[v]\\).
- Subsequent decreases to \\(d[p]\\) preserve the inequality.
- However, subsequent decreases to \\(d[v]\\) also need to be accounted for.

In particular, a later assignment to \\(v\\) changes its parent. Your argument must refer to the *currently saved parent edge*, not an arbitrary historical assignment.

### **2. Why should the extracted cycle be strictly negative?**

This is the most interesting part.

Suppose the extracted cycle is:

\\[ v_0 \to v_1 \to \cdots \to v_k \to v_0. \\]

For each saved parent edge, you want:

\\[ d[v_i] \ge d[v\_{i+1}]+w(v\_{i+1},v_i). \\]

Adding these inequalities gives:

\\[ 0 \ge \sum\_{i=0}^{k} w(v\_{i+1},v_i). \\]

But this only establishes a **nonpositive** cycle.

To establish a strictly negative cycle, you need at least one strict inequality.

Your proposed earliest-last-assignment argument is intended to provide that strict inequality. The critical question is:

- Is the earliest last assignment necessarily followed by a parent assignment that occurred later?
- Does the saved parent still correspond to that later assignment when extraction begins?
- Can a subsequent relaxation overwrite the relevant parent and invalidate the argument?

You need to prove these properties for the actual relaxation order, rather than infer them from the existence of a final-scan update.

### **3. Does early termination preserve the predecessor argument?**

This is where your shared relaxation routine needs particular care.

A routine that:

1. Checks whether an edge can strictly improve its target.
2. Updates the target's distance.
3. Updates its parent.
4. Reports that an update occurred.

can serve both ordinary scans and the explicit check scan.

However, the check scan must stop **after** applying the first improvement, not before it.

Your first example demonstrates why: the first improving target has no parent before its update. Your full-scan implementation obtains a usable predecessor structure through subsequent updates.

The early-stop version needs its own proof that the first applied update already leaves a valid predecessor chain for extraction.

### **What I would test next**

I would focus on a particularly adversarial case:

- A reachable negative cycle.
- A downstream sink whose distance is improved before any cycle vertex is updated during the check scan.
- A predecessor chain that contains several vertices outside the cycle.
- An edge ordering that causes parent assignments to be overwritten.

Your second example already covers the downstream-sink aspect.

**My main challenge to your proof:** You have shown why the cycle's total weight is nonpositive, but you have not yet rigorously established why at least one of the saved-parent inequalities must be strict.

That is the missing step between extracting a predecessor cycle and proving that the printed exploit is genuinely negative.

## Prompt 3

I take your point about referring to the current parent. Let time(v) be the position of v's last successful relaxation at the moment we extract. Distance and parent change together, so if p is the current parent of v then the saved equality was d[v] = d_at_time(v)[p]+w(p,v). Choose the vertex on the extracted cycle with minimum time(v). Its parent is on the same cycle and is distinct because self-loops are invalid. Successful assignments are serial, so time(p) is strictly later, d[p] is strictly lower now, and that inequality is strict. I still need the separate argument that the walk cannot run out of parents. In the parentless-source case that would give a simple source path and contradict the strict update after V−1 scans.

I tested the early-stop build on a longer tail. With arcs (3,4,0),(2,3,0),(0,1,0),(1,2,-3),(2,1,1), after four scans I got [0,-8,-9,-7,-5]. The first improving arc is 3→4, and the predecessor tail starts 4→3→2. The command printed stamina 0 4 exploit 2 1 2, which the checker validated against the directed arcs. Does the timestamp formulation resolve the strictness objection? Which edge case would still distinguish a correct witness from merely finding some repeated vertex?

## Response 3

Yes. Your timestamp formulation resolves the strictness objection, provided you define timestamps as the positions of the **latest successful assignments to each vertex**, and preserve the corresponding distance-parent pair atomically.

The key argument is now much stronger.

### **1. Strict negativity**

For every vertex \\(v\\) on the extracted cycle, let \\(p\\) be its current parent.

At the time of \\(v\\)'s last assignment:

\\[ d[v] = d\_{\text{time}(v)}[p] + w(p,v). \\]

Since distances only decrease:

\\[ d[v] \ge d\_{\text{now}}[p] + w(p,v). \\]

Now choose the cycle vertex \\(v\\) with the earliest last-assignment timestamp.

Its parent \\(p\\) must have been assigned later, because:

- \\(p\\) is also on the cycle.
- \\(p\ne v\\), since self-loops are invalid.
- Successful assignments are serial.
- \\(v\\) has the earliest timestamp among cycle vertices.

Therefore:

\\[ d\_{\text{now}}[p] < d\_{\text{time}(v)}[p]. \\]

Consequently, the inequality for this particular edge is strict.

Summing the inequalities around the cycle gives:

\\[ 0 > \sum\_{(u,v)\in C}w(u,v). \\]

That establishes strictly negative total weight, not merely nonpositive weight.

### **2. The remaining issue: reaching a parentless vertex**

Your parentless-source argument is the right direction.

Suppose following predecessors from the last updated vertex reaches a vertex with no parent.

Because every non-source parent assignment comes from a successful relaxation, following the parent chain backwards eventually produces a simple source-to-target path if no vertex repeats.

A simple path contains at most \\(V-1\\) edges.

After \\(V-1\\) complete scans, the distance to every reachable vertex is no greater than the cost of any simple source path.

But the check scan successfully decreases a distance below that bound.

This is a contradiction.

One detail to make explicit: the check scan must begin after exactly \\(V-1\\) complete scans, and the predecessor information must remain consistent with the corresponding distance labels.

### **3. The edge case I would test next**

I would test a negative self-loop separately, even if your input contract currently excludes self-loops.

For example:

```
N 2
R 0 1 0
R 1 1 -1
S 0 1
```

This tests whether your implementation handles a one-vertex negative cycle.

Your current strictness proof explicitly depends on distinct adjacent cycle vertices. If self-loops are invalid, document that precondition. If they are permitted, your proof needs a separate case: the self-loop itself has strictly negative weight.

I would also test parallel directed arcs with opposite costs, ensuring that parent reconstruction identifies the actual selected arc rather than merely the pair of endpoint IDs.

**My assessment:** Your timestamp argument now provides a convincing route to proving strict negativity. The remaining proof obligations are the parent-chain termination argument and ensuring the reconstructed cycle uses actual directed edges. Your longer-tail experiment is useful evidence that early termination preserves the intended witness in that case, but it does not replace those general arguments.

## Prompt 4

I tested the distinction from a repeated vertex to a negative directed cycle. Opposite input orders of a map with two reachable negative cycles produced exploit 1 2 1 and exploit 4 3 4. The independent checker validated those exact arcs, total weights -3 and -4, and reachability from the source. A directed 0→1→0 cycle with arc costs -1 and +1 gave an ordinary distance of -1 to island 1. An unreachable negative cycle also left the reachable cost 4 and the other destination unreachable. The driver rejects self-routes and a second arc with the same ordered endpoints, so I am keeping those as boundary checks of the input contract rather than claiming the algorithm supports those worlds.

I rebuilt and ran the direct oracle on all 24,576 combinations of three-island graphs, sources and orders. It checks directed negative witnesses, source reachability and finite distances separately. I will write that an explicit final scan only runs if ordinary scans have not already gone quiet. A quiet pass from a reachable state can return early. Does the zero-total case test the correct condition, or is there another assumption of the low-level distance model I should mention in the argument?

## Response 4

Your zero-total test checks an important distinction, and its result is correct.

For the cycle

\\[ 0 \rightarrow 1 \rightarrow 0 \\]

with costs \\(-1\\) and \\(+1\\), the total is zero. This is **not a negative cycle**, so Bellman–Ford must not report an exploit merely because a predecessor cycle exists.

Your test demonstrates why checking for a repeated predecessor vertex alone is insufficient. The cycle must also have strictly negative total weight.

### **Another assumption to document: arithmetic range**

There is an important low-level assumption in your implementation:

**Signed 64-bit arithmetic must not overflow during relaxation.**

Even if every individual route cost fits in `int64_t`, a candidate distance calculated as

\\[ d[u]+w(u,v) \\]

might not.

For example, a sufficiently long path with large positive or negative costs can exceed the representable range.

You should establish one of the following:

- The input constraints guarantee every intermediate candidate distance fits in signed 64-bit arithmetic.
- You perform checked addition and handle out-of-range candidates explicitly.

Also, your infinity sentinel needs special treatment:

- Unreachable vertices must remain at infinity.
- Never relax an edge from an infinity-labelled vertex.
- Do not allow a finite candidate to collide with the infinity sentinel.

These details matter because your correctness argument assumes ordinary integer comparisons and additions.

### **Your final-scan conclusion**

Your proposed wording is sound:

1. Initialize only the source to zero.
2. Perform at most \\(V-1\\) ordinary scans.
3. Return immediately if a complete scan is quiet.
4. If all ordinary scans remain active, perform an explicit check scan.
5. Apply the first strict improvement, recording its parent and distance together.
6. Follow \\(V\\) predecessor links to enter a cycle.
7. Reconstruct the cycle and verify its directed edges.

The important distinction is that the explicit check scan does not need to finish. Its first successful relaxation supplies the starting vertex for predecessor extraction.

### **Suggested report wording**

You could describe the testing this way:

> The Bellman–Ford implementation was tested against an independent oracle across 24,576 combinations of three-island graphs, source vertices and edge orders. The checker independently validated finite distances, source reachability and negative-cycle witnesses, including their directed edges and strictly negative total costs. Additional cases covered zero-total cycles, unreachable negative cycles, multiple reachable negative cycles and downstream vertices updated before cycle vertices.

I would retain one explicit limitation in the correctness argument: the proof assumes relaxation arithmetic does not overflow and that the infinity sentinel cannot be mistaken for a finite distance.

**Overall:** Your early-stop design now has a coherent correctness argument. The timestamp argument establishes strict negativity, the \\(V\\)-link argument establishes cycle entry, and your independent checker tests the distinction between a valid negative-cycle witness and merely finding a repeated vertex.

## Prompt 5

I checked the numeric precondition before closing the Bellman–Ford design. The stamina limits are V≤5,000, E≤50,000 and |w|≤10^9. Even an in-place execution can extend a discovered walk only once per examined edge, so across at most V scans a candidate uses at most VE edges from previous assignments. VE·10^9 is 2.5×10^17, inside signed 64-bit range and below the infinity sentinel. A new command case printed cost 3,000,000,000 along three large arcs, and a negative arc from one unreachable island to another left the destination unreachable.

For the required Dijkstra section I worked out this example before asking for review. The directed arcs are s→a of 2, s→b of 4, b→a of -3, and a→t of 1. Extract-once processes s,a,t,b. Even allowing a's label to drop later, it misses rescanning a→t and leaves 3 instead of 2. My implementation reinserts every strict improvement and both functions actually gave labels 0,1,4,2 when called on this test. I plan to distinguish an extract-once implementation that updates settled labels from one that freezes them. Is the four-vertex minimality claim valid under the former convention, considering every simple shortest path on three vertices has at most two arcs?

## Response 5

Your four-vertex example is valid for demonstrating why extract-once Dijkstra fails on negative edges, but I would qualify the minimality argument.

### **1. Your example**

The directed arcs are:

| Arc          | Cost |
| ------------ | ---- |
| \\(s\to a\\) | 2    |
| \\(s\to b\\) | 4    |
| \\(b\to a\\) | -3   |
| \\(a\to t\\) | 1    |

The extraction order is:

\\[ s,\ a,\ t,\ b \\]

The resulting labels are:

| Vertex  | Extract-once label | Correct distance |
| ------- | ------------------ | ---------------- |
| \\(s\\) | 0                  | 0                |
| \\(a\\) | 1                  | 1                |
| \\(b\\) | 4                  | 4                |
| \\(t\\) | 3                  | 2                |

When \\(b\\) is eventually extracted, its relaxation improves \\(a\\) from 2 to 1. However, \\(a\\) has already been processed, so its outgoing edge is not rescanned.

Your reinsertion implementation correctly processes the improvement and obtains \\(d[t]=2\\).

### **2. Is four vertices minimal?**

Under the convention you specified, **yes, for a counterexample involving a reachable destination whose computed distance is incorrect**, assuming:

- The source is initially extracted correctly.
- Extracted vertices are processed only once.
- Later improvements can change their stored labels.
- Those improvements do not trigger another outgoing-edge scan.
- There are no negative cycles reachable from the source.

Your simple-path argument gives the key insight.

With at most three vertices, a simple path contains at most two edges.

For a vertex \\(t\\) whose shortest path is incorrect, consider the first edge along a shortest path whose relaxation has not been propagated correctly.

The predecessor must have received its final shortest label after it was extracted. But with only three vertices, constructing this ordering requires an earlier extracted intermediate vertex and a later vertex that improves it, leaving insufficient room for a separate destination whose label remains incorrect.

Your four-vertex construction supplies exactly those roles:

1. Source.
2. Intermediate vertex that is extracted too early.
3. Later vertex that improves the intermediate.
4. Destination that is not rescanned.

### **3. One important qualification**

Do not generalize the claim to say that extract-once Dijkstra always produces incorrect results with three vertices.

Also distinguish these two failure modes in your report:

- **Frozen settled labels:** Later improvements are discarded.
- **Mutable settled labels:** Later improvements change stored distances, but outgoing edges are not rescanned.

Your example demonstrates the second, more subtle failure mode. It is stronger because even though the intermediate label is eventually corrected, the destination remains wrong.

### **4. Your numeric-bound argument**

Your Bellman–Ford bound is also useful, with one detail worth making explicit.

Across \\(V\\) in-place scans, a successful relaxation can extend a represented walk by at most one edge. Thus the conservative candidate magnitude bound is:

\\[ VE\cdot 10^9 = 5{,}000 \times 50{,}000 \times 10^9 = 2.5\times10^{17}. \\]

This fits comfortably inside signed 64-bit range, provided the implementation's initialization, sentinel and candidate arithmetic follow the same bound.

**My recommendation:** Keep the four-vertex example and explicitly call it a minimal counterexample under the mutable-label, extract-once convention. That qualification makes your claim precise and prevents readers from confusing it with the frozen-label variant.

## Prompt 6

The three-vertex justification I will use is more direct than counting roles. Every two-edge shortest path s→u→t has its first edge relaxed by s before u expands, so when u expands its label is already low enough to set the optimum for t. This only establishes the mutable-label extract-once claim. With one negative edge x→y and no reachable negative cycle, I think my heap implementation can expand x at its final distance before using that edge, because a walk improving x via x→y would have to return to x through a nonnegative cycle. Once that single negative relaxation has happened, live keys are nondecreasing again. A small instrumented trace gave three improvements to one vertex with only two expansions; the bound comes from the two phases of the algorithm.

For the nonnegative timing task, the largest grid had median scan counts 28 forward, 261 reverse and 136 shuffled, including the quiet scan. Yet the paired shuffle/reverse time ratio was 1.370. I reran a control on the same grid beginning at the final labels. With 32 scans for each order and no successful relaxations, shuffle was still 1.402 of reverse time. I think that says the scan and update counts alone do not explain the timing difference. Does that wording overstate what holding those two metrics fixed can rule out?

## Response 6

Your wording is mostly sound, but there are two places where I would tighten the reasoning: the single-negative-edge heap argument and the interpretation of the timing control.

## **1. Three-vertex minimality**

Your revised argument is better.

With three vertices, a simple shortest path has at most two edges:

\\[ s\rightarrow u\rightarrow t. \\]

The source expands first and relaxes \\(s\to u\\). Since there are no reachable negative cycles, the direct edge \\(s\to u\\) is already a shortest path to \\(u\\): any alternative simple path would require another vertex.

Therefore, when \\(u\\) expands, its label is already optimal, and it can relax the final edge to \\(t\\).

This establishes the three-vertex lower bound for your particular mutable-label, extract-once convention.

I would explicitly retain the assumption that shortest paths exist and are finite, excluding reachable negative cycles.

## **2. One negative edge and heap-based Dijkstra**

Your argument is plausible, but distinguish two properties:

- The heap implementation can expand each vertex more than once.
- Its final distances are correct despite one negative edge.

The absence of reachable negative cycles establishes the second property only with an appropriate argument about the relaxation process.

Your observation about the negative edge is useful:

Before relaxing \\(x\to y\\), all other edges are nonnegative. A shortest path to \\(x\\) cannot benefit from traversing that negative edge and returning to \\(x\\), because that would create a cycle containing the only negative edge, and the remaining edges have nonnegative costs.

Consequently, \\(x\\)'s label is final when it is first expanded.

After that negative relaxation, all subsequently processed edges are nonnegative. However, I would avoid saying that all live heap keys are necessarily globally nondecreasing without specifying exactly which keys you mean. Existing stale entries and newly inserted entries need to be distinguished.

Your instrumented trace is useful evidence for the two-phase interpretation, but the proof should establish that no vertex needs to be expanded indefinitely and that the heap eventually processes every necessary improvement.

## **3. Timing experiment: what does the control establish?**

**Observed results**

# **1.370**

Shuffle / reverse original timing

# **1.402**

Shuffle / reverse fixed-state timing

| Measurement                | Forward | Reverse | Shuffled |
| -------------------------- | ------- | ------- | -------- |
| Original scans             | 28      | 261     | 136      |
| Control scans              | 32      | 32      | 32       |
| Control successful updates | 0       | 0       | 0        |

Your conclusion is correct if phrased carefully.

The fixed-state control holds both scan counts and successful-update counts constant. Yet the shuffle/reverse timing ratio remains 1.402.

This supports the conclusion that **differences in scan counts and successful-update counts alone do not explain the observed timing difference**.

However, it does not establish that scan and update counts have no influence on the original timing.

Other factors could include:

- Edge traversal order and memory locality.
- Branch prediction.
- Cache behavior.
- Different distributions of comparisons and early exits within individual scans.
- Measurement variability.

Also, the control begins at the final labels, so it measures a different workload from the original relaxation process.

### **Suggested report wording**

> The original experiment showed substantial differences in scan counts across edge orders. To investigate whether scan and successful-update counts alone explained the timing differences, I ran a fixed-state control using the same final labels. Each order performed 32 scans with zero successful updates. Shuffle remained 1.402 times as fast as reverse in this control, compared with 1.370 in the original experiment. This indicates that scan and successful-update counts alone are insufficient to explain the observed timing differences. Other order-dependent execution effects may contribute.

**My recommendation:** Keep the distinction between an observed timing difference and a causal explanation. Your control is good evidence against a simple operation-count explanation, but not proof of a particular hardware-level cause.

## Prompt 7

I checked the final 32-scan replay data to get the timing direction right. The labels were already final, with zero successful updates for every order. Shuffle took 1.186250 ms and reverse took 0.846167 ms, so shuffle/reverse is 1.402, meaning shuffle was slower. The original paired ratio was about 1.370 in the same direction. This challenges a counts-only explanation. I will leave the remaining execution effects unidentified because the control changes the relaxation state.

My final route decision is to commit the first strict update on scan V before tracing V predecessors to get inside the cycle and reversing the extracted sequence. The two-edge tail test's first update was 3 to 4, and the program returned the cycle 2 1 2. Opposite input orders on the two-cycle case produced two different oracle-verified witnesses. A zero-total cycle produced a finite result, and a disconnected negative cycle stayed unreachable. With 250,000 vertices, at most that many passes and at most that many edges, multiplication by the maximum magnitude 10^9 gives the conservative 2.5 × 10^17 relaxation bound. For the one-negative-edge analysis I will say that equal or stale heap labels do not re-expand a vertex and that expanded labels, rather than every temporary improvement, are bounded. My test showed three labels proposed for one vertex with two actual expansions. What should I qualify so the scan-V witness, finite arithmetic and performance control are defensible as three distinct claims?

## Response 7

I would present these as three separate claims, each with its own assumptions and evidence. Your conclusions are defensible, but there are a few qualifications worth preserving.

## **1. Scan-V witness correctness**

Your decision to commit the first strict update on scan \\(V\\) before extracting the witness is reasonable.

The correctness argument should establish the following sequence:

1. The first \\(V-1\\) complete scans fail to terminate early, so a reachable negative cycle exists if the next check scan finds a strict improvement.
2. The first successful update on scan \\(V\\) updates the distance and parent together.
3. Following \\(V\\) parent links enters a cycle, rather than merely encountering an arbitrary repeated vertex.
4. The timestamp argument establishes that the extracted cycle has strictly negative total weight.
5. Reversing the predecessor sequence produces the correct directed traversal order.

Your two-edge-tail test is particularly useful because it demonstrates that the first updated target need not belong to the cycle.

One qualification: your proof depends on the check scan starting after exactly \\(V-1\\) complete scans, with no intervening changes to the distance or parent state. Also, the input contract excluding self-routes should be stated because your timestamp strictness argument uses distinct adjacent cycle vertices.

Your different witnesses under different edge orders are not a correctness problem. A negative-cycle witness need not be unique.

## **2. Finite arithmetic**

Your bound is defensible, but state its scope precisely.

For the stamina limits:

\\[ V\leq250{,}000,\qquad E\leq250{,}000, \qquad |w|\leq10^9 \\]

your conservative bound is:

\\[ VE\cdot10^9=2.5\times10^{17}. \\]

This is comfortably within the signed 64-bit range.

I would qualify this in two ways:

- The bound applies to candidate distances generated by the stated relaxation schedule, including the final check scan.
- Infinity must be represented separately from finite distances, or its sentinel value must be proven not to collide with any permitted finite candidate.

Also, make sure the multiplication used to establish the bound is itself evaluated in a sufficiently wide type in your implementation or report calculation.

Your large-arc test yielding 3,000,000,000 is useful supplementary evidence, but it does not replace the general bound.

## **3. Performance control**

Your corrected interpretation is important.

**Fixed-state control**

Shuffle

# **1.186250 ms**

Reverse

# **0.846167 ms**

**Shuffle / reverse**

# **1.402×**

Shuffle was slower

The control supports a specific conclusion:

- Scan counts were identical.
- Successful-update counts were identical.
- Shuffle was still slower.

Therefore, scan and successful-update counts alone cannot explain the timing difference.

But the control begins at already-final labels. It does not preserve the original relaxation dynamics, and it does not isolate cache locality, branch prediction or any other individual execution effect.

I agree with leaving those effects unidentified.

I would also describe 1.402 as an observed ratio, rather than a universal performance characteristic. If you have repeated paired measurements, report the median or distribution and the number of repetitions.

## **4. One-negative-edge heap qualification**

There is one additional distinction I would make in your analysis.

Your statement that equal or stale heap labels do not re-expand a vertex is an implementation property, not a general property of Dijkstra.

Be explicit that your implementation:

- Uses strict improvements only.
- Rejects stale heap entries.
- Does not expand an already-finalized vertex again.
- Allows an improved tentative label to be reinserted when the vertex has not yet been finalized.

Your observation of three proposed labels but two actual expansions illustrates why counting successful relaxations and counting expansions are different metrics.

Also, your one-negative-edge proof should explicitly assume no reachable negative cycle. The fact that all other edges are nonnegative is what makes the argument about the negative edge's source being finalized possible.

## **Suggested final report structure**

I would keep the three conclusions separate:

| Claim                           | Evidence                                                        | Qualification                                                                   |
| ------------------------------- | --------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| Early scan-V witness extraction | Tail cases, multiple cycles, independent directed-cycle checker | Requires synchronized distance-parent updates and valid predecessor-chain proof |
| Signed 64-bit safety            | \\(VE\cdot10^9=2.5\times10^{17}\\)                              | Applies to bounded relaxation schedule and sentinel handling                    |
| Timing differences              | Fixed-state control, 32 scans, zero updates                     | Rejects counts-only explanation, not individual hardware-level causes           |

**My overall assessment:** Your design and experimental reasoning are now substantially more defensible. The most important thing is not to let evidence for one claim silently become evidence for another. The witness checker establishes structural correctness; the arithmetic bound establishes numeric safety; and the fixed-state replay challenges one performance explanation. None needs to claim more than it actually demonstrates.
