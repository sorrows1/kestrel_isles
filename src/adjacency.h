#pragma once

#include "graph.h"

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
