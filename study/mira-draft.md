# Mira's map draft

The Week 08 adjacency list stores neighbor and bridge index at both endpoints. I count two entries per undirected bridge. BFS marks a vertex when enqueued. Every next layer extends a shortest path by one bridge. Starting unmarked roots in ascending order makes each assigned label the smallest reachable island.

I expect an empty graph to give each island its own region and hops from a vertex to itself to be zero. Adding a bridge between two regions should merge them at the next query because the program rebuilds its lists each time. I would try reserving each inner vector by degree if many small allocations dominate.

My output check would require one occurrence of each island and place endpoints of every bridge together. I need evidence about the remaining failure modes and any memory-layout benefit.
