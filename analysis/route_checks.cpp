#include "routes.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>

void require(bool okay) {
  if (!okay)
    throw std::runtime_error("function check failed");
}

void check_routes() {
  constexpr int n = 3;
  constexpr int inf = 1000;
  const std::array<int, 4> weights{inf, -2, 0, 2};

  for (int graph = 0; graph < 4096; ++graph) {
    int bits = graph;
    std::vector<Edge> arcs;
    int cost[n][n], shortest[n][n];
    for (int u = 0; u < n; ++u) {
      for (int v = 0; v < n; ++v) {
        int w = 0;
        if (u != v) {
          w = weights[bits % 4];
          bits /= 4;
          if (w != inf)
            arcs.push_back({u, v, w});
        }
        cost[u][v] = shortest[u][v] = w;
      }
    }
    for (int k = 0; k < n; ++k)
      for (int u = 0; u < n; ++u)
        for (int v = 0; v < n; ++v)
          if (shortest[u][k] != inf && shortest[k][v] != inf)
            shortest[u][v] = std::min(shortest[u][v], shortest[u][k] + shortest[k][v]);
    for (int order = 0; order < 2; ++order) {
      for (int s = 0; s < n; ++s) {
        bool negative = false;
        for (int u = 0; u < n; ++u)
          negative |= shortest[s][u] != inf && shortest[u][u] < 0;
        const auto got = bellman_ford(n, arcs, s);
        require(got.negative_cycle.empty() == !negative);
        if (negative) {
          const auto& cycle = got.negative_cycle;
          require(cycle.size() >= 2 && cycle.size() <= n);
          require(shortest[s][cycle.front()] != inf);
          int total = 0;
          for (std::size_t i = 0; i < cycle.size(); ++i) {
            require(cycle[i] >= 0 && cycle[i] < n);
            require(std::count(cycle.begin(), cycle.end(), cycle[i]) == 1);
          }
          for (std::size_t i = 0; i < cycle.size(); ++i) {
            int w = cost[cycle[i]][cycle[(i + 1) % cycle.size()]];
            require(w != inf);
            total += w;
          }
          require(total < 0);
        } else {
          for (int u = 0; u < n; ++u)
            require(got.paths.dist[u] == (shortest[s][u] == inf ? kUnreachable : shortest[s][u]));
        }
      }
      std::reverse(arcs.begin(), arcs.end());
    }
  }
}

int main() {
  check_routes();

  const std::vector<Edge> adversary{{0, 1, 2}, {0, 2, 4}, {2, 1, -3}, {1, 3, 1}};
  const auto heap = dijkstra(4, adversary, 0);
  const auto bf = bellman_ford(4, adversary, 0);
  require(heap.dist == std::vector<std::int64_t>({0, 1, 4, 2}));
  require(bf.paths.dist == heap.dist);
  std::cout << "One negative edge: Dijkstra and Bellman-Ford both gave 0,1,4,2.\n";

  const std::vector<Edge> hand{{0, 2, -1}, {1, 0, 1}, {1, 2, -3}, {2, 1, 1}};
  const auto cycle = bellman_ford(3, hand, 0).negative_cycle;
  require(cycle == std::vector<int>({1, 2}) || cycle == std::vector<int>({2, 1}));
  std::cout << "24,576 graph/source/order cases passed. "
            << "The hand case gives 1 -> 2 -> 1 (weight -2).\n";
}
