#include "maintenance.h"
#include "routes.h"
#include "disjoint_set.h"
#include "nanobench.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

namespace {
using Edges = std::vector<Edge>;
std::mt19937 rng(2183);
unsigned seed = 2183;
bool reverse_pair = false;
std::ofstream rows;
bool control_only = false;
std::ofstream epochs;
void require(bool ok) {
  if (!ok)
    throw std::runtime_error("cross-check failed");
}

template <class F>
void measure(const std::string& family, int n, const Edges& edges, const std::string& method, F run,
             double metric = 0, double root_links = 0) {
  ankerl::nanobench::Bench b;
  b.output(nullptr).epochs(5).minEpochTime(std::chrono::milliseconds(2));
  b.run(method, [&] { ankerl::nanobench::doNotOptimizeAway(run()); });
  const auto& result = b.results().back();
  using Measure = ankerl::nanobench::Result::Measure;
  for (size_t i = 0; i < result.size(); ++i)
    epochs << family << ',' << n << ',' << seed << ',' << method << ',' << i << ','
           << result.get(i, Measure::elapsed) * 1000 << '\n';
  rows << family << ',' << n << ',' << edges.size() << ',' << seed << ',' << method << ','
       << result.median(Measure::elapsed) * 1000 << ',' << metric << ',' << root_links << std::endl;
}

Edges graph(const std::string& family, int n) {
  Edges edges;
  auto add = [&](int u, int v) { edges.push_back({u, v, 1 + int(rng() % 1000)}); };
  if (family == "grid") {
    int side = static_cast<int>(std::sqrt(n));
    for (int v = 0; v < n; ++v) {
      if (v % side)
        add(v - 1, v);
      if (v >= side)
        add(v - side, v);
    }
  } else if (family == "dense") {
    for (int v = 1; v < n; ++v)
      for (int u = 0; u < v; ++u)
        if (u == v - 1 || (rng() & 1))
          add(u, v);
  } else {
    std::set<std::pair<int, int>> seen;
    auto insert = [&](int a, int b) {
      if (a > b)
        std::swap(a, b);
      if (a != b && seen.emplace(a, b).second)
        add(a, b);
    };
    for (int v = 1; v < n; ++v)
      insert(rng() % v, v);
    while (edges.size() < 4ULL * n) {
      int a = rng() % n;
      int b = rng() % n;
      insert(a, b);
    }
  }
  return edges;
}

template <bool Rank, bool Compress, bool Count> struct Variant {
  std::vector<int> parent, rank;
  std::int64_t links = 0, compression_links = 0, calls = 0;
  explicit Variant(int n) : parent(n), rank(n) {
    std::iota(parent.begin(), parent.end(), 0);
  }
  int find(int x) {
    int root = x;
    if constexpr (Count)
      ++calls;
    while (parent[root] != root) {
      root = parent[root];
      if constexpr (Count)
        ++links;
    }
    if constexpr (Compress) {
      while (x != root) {
        int next = parent[x];
        parent[x] = root;
        x = next;
        if constexpr (Count)
          ++compression_links;
      }
    }
    return root;
  }
  bool unite(int x, int y) {
    int a = find(x), b = find(y);
    if (a == b)
      return false;
    if constexpr (Rank) {
      if (rank[a] < rank[b])
        std::swap(a, b);
      if (rank[a] == rank[b])
        ++rank[a];
    }
    parent[b] = a;
    return true;
  }
};

template <bool Rank, bool Compress, bool Count> auto variant_forest(int n, Edges edges) {
  std::sort(edges.begin(), edges.end(), [](auto a, auto b) { return a.weight < b.weight; });
  Variant<Rank, Compress, Count> sets(n);
  SpanningForest forest;
  for (auto e : edges)
    if (sets.unite(e.from, e.to)) {
      forest.edges.push_back(e);
      forest.total_length += e.weight;
    }
  return std::tuple{forest.total_length,
                    Count ? double(sets.links + sets.compression_links) / sets.calls : 0.0,
                    Count ? double(sets.links) / sets.calls : 0.0};
}
template <bool Rank, bool Compress>
void variant(int n, const Edges& edges, const std::string& name, std::int64_t expected) {
  auto [total, links, root_links] = variant_forest<Rank, Compress, true>(n, edges);
  require(total == expected);
  measure(
      "dsu", n, edges, name,
      [&] { return std::get<0>(variant_forest<Rank, Compress, false>(n, edges)); }, links,
      root_links);
}

int bf_rounds(int n, const Edges& edges, const std::vector<std::int64_t>& expected) {
  std::vector<std::int64_t> dist(n, kUnreachable);
  dist[0] = 0;
  for (int round = 1; round <= n; ++round) {
    bool changed = false;
    for (auto e : edges) {
      if (dist[e.from] != kUnreachable && dist[e.from] + e.weight < dist[e.to]) {
        dist[e.to] = dist[e.from] + e.weight;
        changed = true;
      }
    }
    if (!changed) {
      require(dist == expected);
      return round;
    }
  }
  throw std::runtime_error("unexpected negative cycle");
}

void routes(const std::string& family, int n) {
  Edges arcs;
  for (auto e : graph(family, n)) {
    arcs.push_back(e);
    arcs.push_back({e.to, e.from, e.weight});
  }

  auto expected = dijkstra(n, arcs, 0).dist;

  for (std::string order : {"forward", "reverse", "shuffle"}) {
    if (order == "reverse")
      std::reverse(arcs.begin(), arcs.end());
    if (order == "shuffle")
      std::shuffle(arcs.begin(), arcs.end(), rng);
    auto label = "route-" + family + "-" + order;
    if (control_only) {
      if (family == "grid" && n == 16384) {

        measure(label, n, arcs, "fixed32", [&] {
          auto dist = expected;
          for (int scan = 0; scan < 32; ++scan) {
            for (auto e : arcs)
              if (dist[e.from] + e.weight < dist[e.to])
                dist[e.to] = dist[e.from] + e.weight;
            ankerl::nanobench::doNotOptimizeAway(dist.data());
          }
          return dist.back();
        });
      }
      continue;
    }
    auto bf = bellman_ford(n, arcs, 0);
    require(bf.negative_cycle.empty() && bf.paths.dist == expected);
    require(dijkstra(n, arcs, 0).dist == expected);
    const auto dijkstra_run = [&] {
      measure(label, n, arcs, "Dijkstra", [&] { return dijkstra(n, arcs, 0).dist.back(); });
    };
    const auto bf_run = [&] {
      measure(
          label, n, arcs, "BF", [&] { return bellman_ford(n, arcs, 0).paths.dist.back(); },
          bf_rounds(n, arcs, expected));
    };
    if (reverse_pair) {
      bf_run();
      dijkstra_run();
    } else {
      dijkstra_run();
      bf_run();
    }
  }
}
}

void study_seed() {
  rng.seed(seed);
  for (std::string family : {"sparse", "dense", "grid"}) {
    auto sizes = family == "dense" ? std::vector{512, 1024, 2048, 4096}
                                   : std::vector{4096, 16384, 65536, 262144};
    for (int n : sizes) {
      auto edges = graph(family, n);
      if (control_only)
        continue;
      require(prim(n, edges).total_length == kruskal(n, edges).total_length);
      const auto k = [&] {
        measure(family, n, edges, "Kruskal", [&] { return kruskal(n, edges).total_length; });
      };
      const auto p = [&] {
        measure(family, n, edges, "Prim", [&] { return prim(n, edges).total_length; });
      };
      if (reverse_pair) {
        p();
        k();
      } else {
        k();
        p();
      }
    }
  }
  for (int n : {512, 1024, 2048, 4096}) {
    if (control_only)
      continue;
    Edges edges;
    auto add = [&](int u, int v) { edges.push_back({u, v, static_cast<int>(edges.size()) + 1}); };

    for (int stride = 1; stride < 16; stride *= 2)
      for (int u = 0; u < n; u += 2 * stride)
        add(u, u + stride);
    for (int u = 16; u < n; u += 16)
      add(u, u - 16);
    for (int v = 0; v < n / 2; ++v)
      add(v, v + n / 2);
    std::vector<int> names(n);
    std::iota(names.begin(), names.end(), 0);
    std::mt19937 names_rng(seed + n);
    std::shuffle(names.begin(), names.end(), names_rng);
    for (auto& e : edges) {
      e.from = names[e.from];
      e.to = names[e.to];
    }
    auto total = kruskal(n, edges).total_length;
    const auto both = [&] { variant<true, true>(n, edges, "both", total); };
    const auto rank = [&] { variant<true, false>(n, edges, "rank", total); };
    const auto compression = [&] { variant<false, true>(n, edges, "compression", total); };
    const auto neither = [&] { variant<false, false>(n, edges, "neither", total); };
    if (reverse_pair) {
      neither();
      compression();
      rank();
      both();
    } else {
      both();
      rank();
      compression();
      neither();
    }
  }
  for (std::string family : {"sparse", "grid"})
    for (int n : {256, 1024, 4096, 16384})
      routes(family, n);
}

void report_revisited_leaf() {
  Variant<true, false, true> initial(64);
  for (int stride = 1; stride < 64; stride *= 2)
    for (int v = 0; v < 64; v += 2 * stride)
      initial.unite(v, v + stride);
  auto query = [&](auto sets, const char* name, int repeats) {
    sets.parent = initial.parent;
    int changed = 0;
    for (int i = 0; i < repeats; ++i) {
      const auto previous = sets.parent;
      require(sets.find(63) == 0);
      for (int v = 0; v < 64; ++v)
        changed += sets.parent[v] != previous[v];
    }
    const auto average = double(sets.links + sets.compression_links) / sets.calls;
    std::cout << name << ',' << sets.calls << ',' << sets.links << ',' << sets.compression_links
              << ',' << changed << ',' << average << '\n';
  };

  std::cout << "method,finds,root_links,compression_links,changed_parents,links_per_find\n";
  for (int count : {1, 2, 16, 64}) {
    query(Variant<true, false, true>(64), "rank", count);
    query(Variant<true, true, true>(64), "both", count);
  }
}

int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--revisits") {
    report_revisited_leaf();
    return 0;
  }
  control_only = argc == 2 && std::string(argv[1]) == "--control";

  rows.open(control_only ? "results/control.csv" : "results/timings.csv");
  epochs.open(control_only ? "results/control_epochs.csv" : "results/epochs.csv");
  if (!rows || !epochs)
    throw std::runtime_error("create results/ and run from project root");
  rows << "family,n,m,seed,method,ms,metric,root_links_per_find\n";
  epochs << "family,n,seed,method,epoch,ms\n";
  for (auto chosen : {2183U, 6211U, 9011U, 10007U, 16661U}) {
    seed = chosen;
    study_seed();
    if (control_only)
      break;
    reverse_pair = !reverse_pair;
  }
}
