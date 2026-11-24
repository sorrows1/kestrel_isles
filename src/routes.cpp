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
  int last = -1;

  for (int pass = 0; pass < n; ++pass) {
    last = -1;
    for (const auto& e : edges) {
      if (paths.dist[e.from] == kUnreachable)
        continue;
      auto candidate = paths.dist[e.from] + e.weight;
      if (candidate < paths.dist[e.to]) {
        paths.dist[e.to] = candidate;
        paths.parent[e.to] = e.from;
        last = e.to;
      }
    }
    if (last == -1)
      return {std::move(paths), {}};
  }

  for (int i = 0; i < n; ++i)
    last = paths.parent[last];
  std::vector<int> cycle{last};
  for (int u = paths.parent[last]; u != last; u = paths.parent[u])
    cycle.push_back(u);
  std::reverse(cycle.begin(), cycle.end());
  return {{}, std::move(cycle)};
}
