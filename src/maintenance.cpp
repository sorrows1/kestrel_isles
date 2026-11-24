#include "maintenance.h"
#include "adjacency.h"
#include "disjoint_set.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <tuple>

SpanningForest kruskal(int n, const std::vector<Edge>& bridges) {
  auto sorted = bridges;
  std::sort(sorted.begin(), sorted.end(), [](auto a, auto b) { return a.weight < b.weight; });

  DisjointSet sets(n);
  SpanningForest forest;

  for (const auto& e : sorted) {
    if (sets.unite(e.from, e.to)) {
      forest.edges.push_back(e);
      forest.total_length += e.weight;
    }
  }

  return forest;
}

SpanningForest prim(int n, const std::vector<Edge>& bridges) {
  auto adj = adjacency(n, bridges, true);
  using Entry = std::tuple<std::int64_t, int, int>;
  std::priority_queue<Entry, std::vector<Entry>, std::greater<>> heap;
  std::vector<char> used(n);
  SpanningForest forest;

  for (int root = 0; root < n; ++root) {
    if (used[root])
      continue;
    heap.emplace(0, root, -1);
    while (!heap.empty()) {
      auto [length, u, index] = heap.top();
      heap.pop();
      if (used[u])
        continue;
      used[u] = true;
      if (index >= 0) {
        forest.edges.push_back(bridges[index]);
        forest.total_length += length;
      }

      for (auto [v, i] : adj[u]) {
        auto w = bridges[i].weight;
        if (!used[v]) {
          heap.emplace(w, v, i);
        }
      }
    }
  }

  return forest;
}
