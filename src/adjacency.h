#pragma once

#include "graph.h"
#include "packed_lists.h"

#include <numeric>
#include <vector>

struct Neighbor {
  int vertex;
  int edge;
};

inline std::vector<std::vector<Neighbor>> adjacency(int n, const std::vector<Edge>& edges,
                                                    bool undirected) {
  std::vector<std::vector<Neighbor>> adj(n);

  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    const auto& e = edges[i];
    adj[e.from].push_back({e.to, i});
    if (undirected)
      adj[e.to].push_back({e.from, i});
  }

  return adj;
}

inline PackedLists<Neighbor> packed_adjacency(int n, const std::vector<Edge>& edges,
                                              bool undirected) {
  PackedLists<Neighbor> adj(n);

  for (const auto& e : edges) {
    ++adj.offset[e.from + 1];
    if (undirected)
      ++adj.offset[e.to + 1];
  }

  std::partial_sum(adj.offset.begin(), adj.offset.end(), adj.offset.begin());
  adj.items.resize(adj.offset.back());
  auto next = adj.offset;

  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    const auto& e = edges[i];
    adj.items[next[e.from]++] = {e.to, i};
    if (undirected)
      adj.items[next[e.to]++] = {e.from, i};
  }

  return adj;
}
