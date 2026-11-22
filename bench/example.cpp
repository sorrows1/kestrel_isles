// An example benchmark with nanobench (https://nanobench.ankerl.com).
//
// Build and run it with
//
//   make bench
//   build/example        (build\example.exe in PowerShell)
//
// It times kruskal() and prim() on random connected graphs of two densities:
// sparse graphs with about 4n bridges and dense graphs with about n^2 / 4
// bridges. Each measurement computes one spanning forest of a graph that is
// built once, outside the timed code. Until your functions work, the times
// are those of the stubs and mean nothing. Copy this file to write your own
// benchmarks: every .cpp file in bench/ becomes a separate program in build/.
// Do not define ANKERL_NANOBENCH_IMPLEMENT; the Makefile already links the
// library into every benchmark program.

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "graph.h"
#include "maintenance.h"
#include "nanobench.h"

namespace {

// Returns a connected graph with n islands and about m bridges of random
// length: a random spanning tree, so that the graph is connected, plus
// m - (n - 1) bridges between random pairs of islands.
std::vector<Edge> random_connected_graph(int n, std::size_t m,
                                         std::mt19937_64 &rng) {
  std::uniform_int_distribution<std::int64_t> length(1, 1'000'000);
  std::vector<Edge> bridges;
  bridges.reserve(m);
  for (int v = 1; v < n; ++v) {
    std::uniform_int_distribution<int> earlier(0, v - 1);
    bridges.push_back({earlier(rng), v, length(rng)});
  }
  std::uniform_int_distribution<int> island(0, n - 1);
  while (bridges.size() < m) {
    bridges.push_back({island(rng), island(rng), length(rng)});
  }
  return bridges;
}

// Times one spanning-forest function on one graph and adds the result to
// bench.
template <typename Function>
void benchmark_forest(ankerl::nanobench::Bench &bench, const std::string &name,
                      Function forest, int n,
                      const std::vector<Edge> &bridges) {
  bench.run(name + ", n = " + std::to_string(n) +
                ", m = " + std::to_string(bridges.size()),
            [&] {
              ankerl::nanobench::doNotOptimizeAway(
                  forest(n, bridges).total_length);
            });
}

}  // namespace

int main() {
  // Every measurement repeats the computation for at least 10 ms, so that
  // small graphs give stable results, and nanobench reports the median of 11
  // measurements.
  ankerl::nanobench::Bench bench;
  bench.title("Minimum spanning forest")
      .unit("forest")
      .epochs(11)
      .minEpochTime(std::chrono::milliseconds(10));

  std::mt19937_64 rng(2183);
  for (const int n : {1'024, 2'048, 4'096}) {
    const std::vector<Edge> sparse =
        random_connected_graph(n, std::size_t{4} * n, rng);
    const std::vector<Edge> dense = random_connected_graph(
        n, static_cast<std::size_t>(n) * n / 4, rng);

    benchmark_forest(bench, "kruskal, sparse", kruskal, n, sparse);
    benchmark_forest(bench, "prim, sparse", prim, n, sparse);
    benchmark_forest(bench, "kruskal, dense", kruskal, n, dense);
    benchmark_forest(bench, "prim, dense", prim, n, dense);
  }
}
