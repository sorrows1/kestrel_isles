# Jonah's forest draft

Week 09 Kruskal adds a sorted edge only when the endpoint roots differ. For Prim I put all bridges from an added vertex to outside vertices in the queue and skip entries for joined vertices. The lightest bridge leaving the current tree is safe by the cut property. An empty queue ends that component, and an isolated root adds no bridge.

For a tied triangle and an isolated island, different optimal edge lists should both be accepted. My first checker would match each chosen bridge to the original graph and independently compute the optimal sum. The disconnected example should report two chosen bridges on four vertices.

My experiment will use a common input for the two algorithms. I will check which part of the queue work differs and distinguish links needed to find the root from follow-up compression work if the initial metric is incomplete.
