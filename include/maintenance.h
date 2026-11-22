// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.
//
// Week 09: minimum spanning forests of the bridge network.

#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <cstdint>
#include <vector>

#include "graph.h"

// A set of bridges and the sum of their lengths.
struct SpanningForest {
  std::int64_t total_length = 0;
  std::vector<Edge> edges;
};

// Returns a minimum spanning forest of the undirected graph with islands
// 0, ..., n - 1 and the given bridges: a set of bridges, copied unchanged from
// `bridges`, that contains no cycle, connects every pair of islands that the
// whole network connects, and has the smallest total length of all such sets.
// Use Kruskal's algorithm with DisjointSet.
SpanningForest kruskal(int n, const std::vector<Edge> &bridges);

// Returns a minimum spanning forest, as kruskal does, but computed with
// Prim's algorithm and a binary heap, started once in every region.
SpanningForest prim(int n, const std::vector<Edge> &bridges);

#endif  // MAINTENANCE_H
