#include "map_audit.h"
#include "adjacency.h"

#include <queue>

std::vector<int> region_labels(int n, const std::vector<Edge>& bridges) {
  auto adj = packed_adjacency(n, bridges, true);
  std::vector<int> labels(n, -1);
  std::queue<int> pending;

  for (int root = 0; root < n; ++root) {
    if (labels[root] != -1)
      continue;
    labels[root] = root;
    pending.push(root);

    while (!pending.empty()) {
      int u = pending.front();
      pending.pop();

      for (auto neighbor : adj[u]) {
        int v = neighbor.vertex;
        if (labels[v] != -1)
          continue;
        labels[v] = root;
        pending.push(v);
      }
    }
  }

  return labels;
}

int fewest_hops(int n, const std::vector<Edge>& bridges, int s, int t) {
  if (s == t)
    return 0;
  auto adj = packed_adjacency(n, bridges, true);
  std::vector<int> depth(n, -1);
  std::queue<int> pending;
  pending.push(s);
  depth[s] = 0;

  while (!pending.empty()) {
    int u = pending.front();
    pending.pop();

    for (auto neighbor : adj[u]) {
      int v = neighbor.vertex;
      if (depth[v] != -1)
        continue;
      depth[v] = depth[u] + 1;
      if (v == t)
        return depth[v];
      pending.push(v);
    }
  }

  return -1;
}
