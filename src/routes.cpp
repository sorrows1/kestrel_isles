#include "routes.h"
#include "adjacency.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <utility>

ShortestPaths dijkstra(int n, const std::vector<Edge>& edges, int source) {
  auto adj = adjacency(n, edges, false);
  ShortestPaths paths{std::vector<std::int64_t>(n, kUnreachable), std::vector<int>(n, -1)};
  using Entry = std::pair<std::int64_t, int>;
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> heap;

  paths.dist[source] = 0;
  heap.emplace(0, source);

  while (!heap.empty()) {
    auto [distance, u] = heap.top();
    heap.pop();
    if (distance != paths.dist[u])
      continue;
    for (auto [v, i] : adj[u]) {
      auto candidate = distance + edges[i].weight;
      if (candidate < paths.dist[v]) {
        paths.dist[v] = candidate;
        paths.parent[v] = u;
        heap.emplace(candidate, v);
      }
    }
  }

  return paths;
}

BellmanFordResult bellman_ford(int n, const std::vector<Edge>& edges, int source) {
  ShortestPaths paths{std::vector<std::int64_t>(n, kUnreachable), std::vector<int>(n, -1)};

  paths.dist[source] = 0;

  const auto relax = [&](const Edge& edge) {
    if (paths.dist[edge.from] == kUnreachable)
      return false;
    auto candidate = paths.dist[edge.from] + edge.weight;
    if (candidate >= paths.dist[edge.to])
      return false;
    paths.dist[edge.to] = candidate;
    paths.parent[edge.to] = edge.from;
    return true;
  };

  for (int pass = 1; pass < n; ++pass) {
    bool changed = false;
    for (const auto& edge : edges) {
      if (relax(edge))
        changed = true;
    }
    if (!changed)
      return {std::move(paths), {}};
  }

  for (const auto& edge : edges) {

    if (!relax(edge))
      continue;
    int on_cycle = edge.to;
    for (int i = 0; i < n; ++i)
      on_cycle = paths.parent[on_cycle];
    std::vector<int> cycle{on_cycle};
    for (int u = paths.parent[on_cycle]; u != on_cycle; u = paths.parent[u]) {
      cycle.push_back(u);
    }
    std::reverse(cycle.begin(), cycle.end());
    return {{}, std::move(cycle)};
  }

  return {std::move(paths), {}};
}
