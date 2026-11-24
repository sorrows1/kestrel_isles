
#include "../src/adjacency.h"
#include "map_audit.h"
#include "maintenance.h"
#include "quests.h"
#include "nanobench.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>

namespace {
using Edges = std::vector<Edge>;
using Nested = std::vector<std::vector<Neighbor>>;
std::ofstream timings("results/layout_epochs.csv");
std::ofstream heaps("results/prim_operations.csv");

void check(bool condition) {
  if (!condition)
    throw std::runtime_error("layout experiment cross-check failed");
}

template <typename Function>
void measure(std::string_view graph, unsigned seed, std::string_view variant,
             std::string_view phase, Function run) {
  using namespace ankerl::nanobench;
  Bench bench;
  bench.output(nullptr).epochs(7).minEpochTime(std::chrono::milliseconds(3));
  bench.run(std::string(phase), [&] { doNotOptimizeAway(run()); });
  const auto& sample = bench.results().front();
  for (std::size_t i = 0; i < sample.size(); ++i) {
    timings << graph << ',' << seed << ',' << variant << ',' << phase << ',' << i << ','
            << sample.get(i, Result::Measure::elapsed) * 1000 << '\n';
  }
}

Edges make_edges(std::string_view graph, int n, unsigned seed) {
  Edges edges;
  std::mt19937 rng(seed);
  for (int v = 1; v < n; ++v) {
    edges.push_back({v - 1, v, 1 + static_cast<int>(rng() % 1000)});
  }
  if (graph == "dense") {
    for (int v = 2; v < n; ++v)
      for (int u = 0; u < v - 1; ++u)
        if (rng() & 1)
          edges.push_back({u, v, 1 + static_cast<int>(rng() % 1000)});
  }
  return edges;
}

Nested nested_graph(int n, const Edges& edges, bool reserve) {
  Nested lists(n);
  if (reserve) {
    std::vector<int> degree(n);
    for (const auto& e : edges) {
      ++degree[e.from];
      ++degree[e.to];
    }
    for (int v = 0; v < n; ++v)
      lists[v].reserve(degree[v]);
  }
  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    const auto& e = edges[i];
    lists[e.from].push_back({e.to, i});
    lists[e.to].push_back({e.from, i});
  }
  return lists;
}

template <typename Lists> std::vector<int> labels(int n, const Lists& adj) {
  std::vector<int> result(n, -1);
  std::queue<int> queue;

  for (int root = 0; root < n; ++root) {
    if (result[root] != -1)
      continue;
    result[root] = root;
    queue.push(root);
    while (!queue.empty()) {
      int u = queue.front();
      queue.pop();
      for (auto arc : adj[u]) {
        if (result[arc.vertex] != -1)
          continue;
        result[arc.vertex] = root;
        queue.push(arc.vertex);
      }
    }
  }
  return result;
}

template <typename Build>
void layout(std::string_view graph, unsigned seed, std::string_view variant, int n, Build build,
            const std::vector<int>& expected) {
  const auto adj = build();
  check(labels(n, adj) == expected);
  measure(graph, seed, variant, "build", [&] {
    auto result = build();
    ankerl::nanobench::doNotOptimizeAway(result);
    return result[n / 2].size();
  });
  measure(graph, seed, variant, "traverse", [&] { return labels(n, adj).back(); });
  measure(graph, seed, variant, "combined", [&] { return labels(n, build()).back(); });
}

struct HeapCounts {
  std::size_t pushes = 0, stale = 0, peak = 0, scans = 0, comparisons = 0;
};

void check_packed_records(int n, const Edges& edges, bool undirected) {
  const auto ordinary = adjacency(n, edges, undirected);
  const auto packed = packed_adjacency(n, edges, undirected);
  check(packed.offset.size() == static_cast<std::size_t>(n) + 1);
  check(packed.offset.front() == 0 && packed.offset.back() == packed.items.size());
  check(packed.items.size() == (undirected ? 2 : 1) * edges.size());
  for (int v = 0; v < n; ++v) {
    const auto row = packed[v];
    check(std::equal(
        row.begin(), row.end(), ordinary[v].begin(), ordinary[v].end(),
        [](Neighbor a, Neighbor b) { return a.vertex == b.vertex && a.edge == b.edge; }));
  }
}

template <bool BestKey, bool Count>
auto prim_trial(int n, const PackedLists<Neighbor>& adj, const Edges& edges) {
  using Entry = std::tuple<std::int64_t, int>;
  HeapCounts counts;
  const auto later = [&](const Entry& a, const Entry& b) {
    if constexpr (Count)
      ++counts.comparisons;
    return a > b;
  };
  std::priority_queue<Entry, std::vector<Entry>, decltype(later)> pending(later);
  std::vector<char> used(n);
  std::vector<std::int64_t> key;
  if constexpr (BestKey)
    key.assign(n, kUnreachable);
  std::int64_t total = 0;

  auto expand = [&](int u) {
    used[u] = true;
    for (const auto [v, i] : adj[u]) {
      if constexpr (Count)
        ++counts.scans;
      if (used[v])
        continue;
      auto w = edges[i].weight;
      if constexpr (BestKey) {
        if (w >= key[v])
          continue;
        key[v] = w;
      }
      pending.emplace(w, v);
      if constexpr (Count) {
        ++counts.pushes;
        counts.peak = std::max(counts.peak, pending.size());
      }
    }
  };

  for (int root = 0; root < n; ++root) {
    if (used[root])
      continue;
    expand(root);
    while (!pending.empty()) {
      auto [length, u] = pending.top();
      pending.pop();
      if (used[u]) {
        if constexpr (Count)
          ++counts.stale;
        continue;
      }
      total += length;
      expand(u);
    }
  }
  return std::pair{total, counts};
}

QuestSchedule nested_quests(const std::vector<std::int64_t>& duration,
                            const std::vector<Prerequisite>& edges) {
  const int n = static_cast<int>(duration.size());
  std::vector<std::vector<int>> adj(n);
  std::vector<int> indegree(n);
  for (auto [u, v] : edges) {
    adj[u].push_back(v);
    ++indegree[v];
  }
  std::queue<int> ready;
  for (int u = 0; u < n; ++u)
    if (!indegree[u])
      ready.push(u);
  QuestSchedule result{duration, std::vector<int>(n, -1)};
  while (!ready.empty()) {
    int u = ready.front();
    ready.pop();
    for (int v : adj[u]) {
      auto finish = result.finish[u] + duration[v];
      if (result.critical[v] == -1 || finish > result.finish[v]) {
        result.finish[v] = finish;
        result.critical[v] = u;
      }
      if (--indegree[v] == 0)
        ready.push(v);
    }
  }
  return result;
}
}

int main() {
  if (!timings || !heaps)
    throw std::runtime_error("run from project root with results/");
  check_packed_records(6, {{0, 2, 9}, {1, 2, 0}, {2, 4, 3}}, true);
  check_packed_records(6, {{0, 2, 9}, {1, 2, 0}, {2, 4, 3}}, false);
  check_packed_records(4, {}, true);
  std::cout << "Verified ordered packed records and original edge indices on three small inputs.\n";
  timings << "graph,seed,variant,phase,epoch,ms\n";
  heaps << "graph,seed,variant,pushes,stale_pops,peak_entries,edge_scans,heap_comparisons\n";

  for (const auto& [graph, n] :
       std::vector<std::pair<std::string, int>>{{"chain", 200000}, {"dense", 1024}}) {
    for (unsigned seed : {2183, 6211, 9011}) {
      const auto edges = make_edges(graph, n, seed);
      const auto expected = region_labels(n, edges);
      const auto plain = [&] {
        layout(graph, seed, "plain", n, [&] { return nested_graph(n, edges, false); }, expected);
      };
      const auto packed = [&] {
        layout(
            graph, seed, "packed", n, [&] { return packed_adjacency(n, edges, true); }, expected);
      };
      if (seed == 6211)
        packed();
      else
        plain();
      layout(graph, seed, "reserved", n, [&] { return nested_graph(n, edges, true); }, expected);
      if (seed == 6211)
        plain();
      else
        packed();
      const auto adj = packed_adjacency(n, edges, true);
      const auto optimum = kruskal(n, edges).total_length;
      auto [all_total, all] = prim_trial<false, true>(n, adj, edges);
      auto [key_total, key] = prim_trial<true, true>(n, adj, edges);
      check(all_total == optimum && key_total == optimum);
      for (const auto& [name, c] : std::array{std::pair{"all", all}, std::pair{"best", key}}) {
        heaps << graph << ',' << seed << ',' << name << ',' << c.pushes << ',' << c.stale << ','
              << c.peak << ',' << c.scans << ',' << c.comparisons << '\n';
      }
      const auto a = [&] {
        measure(graph, seed, "all", "heap",
                [&] { return prim_trial<false, false>(n, adj, edges).first; });
      };
      const auto k = [&] {
        measure(graph, seed, "best", "heap",
                [&] { return prim_trial<true, false>(n, adj, edges).first; });
      };
      if (seed == 6211) {
        k();
        a();
      } else {
        a();
        k();
      }
    }
  }
  constexpr int quests = 200000;

  std::vector<Prerequisite> dependencies;
  for (int v = 1; v < quests; ++v)
    for (int jump = 1; jump <= std::min(v, 5); ++jump)
      dependencies.push_back({v - jump, v});
  for (unsigned seed : {2183, 6211, 9011}) {
    std::mt19937 random(seed);
    std::vector<std::int64_t> duration(quests);
    for (auto& time : duration)
      time = random() % 6;
    auto original = nested_quests(duration, dependencies);
    auto revised = quest_schedule(duration, dependencies);
    check(revised && revised->finish == original.finish && revised->critical == original.critical);
    check(revised->finish.back() ==
          std::accumulate(duration.begin(), duration.end(), std::int64_t{0}));
    const auto a = [&] {
      measure("quest-chain", seed, "plain", "combined",
              [&] { return nested_quests(duration, dependencies).finish.back(); });
    };
    const auto p = [&] {
      measure("quest-chain", seed, "packed", "combined",
              [&] { return quest_schedule(duration, dependencies)->finish.back(); });
    };
    if (seed == 6211) {
      p();
      a();
    } else {
      a();
      p();
    }
  }
}
