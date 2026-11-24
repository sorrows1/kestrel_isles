
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
std::ofstream timings, heaps;

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
  if (graph == "star-isolates") {
    for (int v = 1; v < n; ++v)
      if (v % 8 != 0)
        edges.push_back({0, v, 1 + static_cast<int>(rng() % 1000)});
    return edges;
  }
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

bool matching_records(int n, const Edges& edges, bool undirected,
                      const PackedLists<Neighbor>& packed) {
  const auto ordinary = adjacency(n, edges, undirected);
  if (packed.offset.size() != static_cast<std::size_t>(n) + 1 || packed.offset.front() != 0 ||
      packed.offset.back() != packed.items.size() ||
      !std::is_sorted(packed.offset.begin(), packed.offset.end()) ||
      packed.items.size() != (undirected ? 2 : 1) * edges.size())
    return false;
  for (int v = 0; v < n; ++v) {
    const auto row = packed[v];
    if (!std::equal(
            row.begin(), row.end(), ordinary[v].begin(), ordinary[v].end(),
            [](Neighbor a, Neighbor b) { return a.vertex == b.vertex && a.edge == b.edge; }))
      return false;
  }
  return true;
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

void report_storage() {
  const int n = 200000;
  std::cout << "graph,plain_reserved_bytes,degree_reserved_bytes,packed_reserved_bytes\n";
  for (auto graph : {"chain", "star-isolates"}) {
    const auto edges = make_edges(graph, n, 2183);
    auto nested_bytes = [](const Nested& lists) {
      auto total = lists.capacity() * sizeof(Nested::value_type);
      for (const auto& line : lists)
        total += line.capacity() * sizeof(Neighbor);
      return total;
    };
    const auto packed = packed_adjacency(n, edges, true);
    const auto bytes =
        packed.offset.capacity() * sizeof(std::size_t) + packed.items.capacity() * sizeof(Neighbor);
    std::cout << graph << ',' << nested_bytes(adjacency(n, edges, true)) << ','
              << nested_bytes(nested_graph(n, edges, true)) << ',' << bytes << '\n';
  }
}

int main(int argc, char** argv) {
  if (argc == 2 && std::string_view(argv[1]) == "--storage") {
    report_storage();
    return 0;
  }
  timings.open("results/layout_epochs.csv");
  heaps.open("results/prim_operations.csv");
  if (!timings || !heaps)
    throw std::runtime_error("run from project root with results/");
  const Edges sample{{0, 2, 9}, {1, 2, 0}, {2, 4, 3}};
  auto records = packed_adjacency(6, sample, true);
  check(matching_records(6, sample, true, records));
  check(matching_records(6, sample, false, packed_adjacency(6, sample, false)));
  check(matching_records(4, {}, true, packed_adjacency(4, {}, true)));
  auto original_labels = labels(6, records);
  records.items[0].edge = 1;
  check(labels(6, records) == original_labels);
  check(!matching_records(6, sample, true, records));
  std::cout << "Verified ordered packed records and original edge indices on three small inputs.\n";
  std::cout << "Rejected a wrong edge index that left every region label unchanged.\n";
  timings << "graph,seed,variant,phase,epoch,ms\n";
  heaps << "graph,seed,variant,pushes,stale_pops,peak_entries,edge_scans,heap_comparisons\n";

  for (const auto& [graph, n] : std::vector<std::pair<std::string, int>>{
           {"chain", 200000}, {"dense", 1024}, {"star-isolates", 200000}}) {
    for (unsigned seed : {2183, 6211, 9011}) {
      const auto edges = make_edges(graph, n, seed);
      const auto expected = region_labels(n, edges);
      if (graph == "star-isolates") {
        for (int v = 1; v < n; ++v)
          check(expected[v] == (v % 8 == 0 ? v : 0));
      }
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
      if (graph == "star-isolates")
        continue;
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
