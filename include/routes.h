// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.
//
// Week 10: single-source shortest paths on directed graphs.

#ifndef ROUTES_H
#define ROUTES_H

#include <cstdint>
#include <vector>

#include "graph.h"

// Shortest-path distances and a shortest-path tree from one source. Both
// vectors have size n. dist[v] is the length of a shortest path from the
// source to v, or kUnreachable. parent[v] is -1 for the source and for every
// unreachable vertex; for every other vertex v, parent[v] is a vertex u such
// that an edge from u to v has weight dist[v] - dist[u].
struct ShortestPaths {
  std::vector<std::int64_t> dist;
  std::vector<int> parent;
};

// Returns the shortest paths from `source` in the directed graph with
// vertices 0, ..., n - 1 and the given edges, all of nonnegative weight.
// Use Dijkstra's algorithm with a binary heap.
ShortestPaths dijkstra(int n, const std::vector<Edge> &edges, int source);

// The result of the Bellman-Ford algorithm. If a cycle of negative total
// weight can be reached from the source, negative_cycle lists its vertices
// v0, v1, ..., vk in order, so that the graph has edges v0 -> v1, ...,
// vk-1 -> vk and vk -> v0 whose weights sum to less than 0, and `paths` is
// unspecified. Otherwise negative_cycle is empty and `paths` holds the
// shortest paths.
struct BellmanFordResult {
  ShortestPaths paths;
  std::vector<int> negative_cycle;
};

// Returns the shortest paths from `source` in the directed graph with
// vertices 0, ..., n - 1 and the given edges, whose weights may be negative,
// or a negative cycle reachable from `source`. Use the Bellman-Ford
// algorithm.
BellmanFordResult bellman_ford(int n, const std::vector<Edge> &edges,
                               int source);

#endif  // ROUTES_H
