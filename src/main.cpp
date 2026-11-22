// Reads a command file that describes a game world and answers its queries:
//
//   ./isles <command_file>
//
// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.
//
// This driver parses the command files, checks that every command is valid,
// calls the functions declared in include/, and writes their results. It also
// checks the shape of every result, such as the sizes of vectors and the
// paths that `parent` and `critical` describe, and stops with an error
// message on standard error if a result is malformed.

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "events.h"
#include "graph.h"
#include "maintenance.h"
#include "map_audit.h"
#include "quests.h"
#include "routes.h"

namespace {

// Largest numbers of islands and quests the driver accepts.
constexpr std::int64_t kMaxCount = 10'000'000;
// Largest absolute value of a bridge length, route cost, or quest duration.
constexpr std::int64_t kMaxWeight = 1'000'000'000;
// Largest event time.
constexpr std::int64_t kMaxTime = 1'000'000'000'000;

// Everything the command file has defined since the last N command.
struct World {
  bool defined = false;
  int islands = 0;
  std::vector<Edge> bridges;
  std::vector<Edge> routes;
  std::vector<std::int64_t> duration;
  std::vector<Prerequisite> prerequisites;
  std::vector<Event> events;
  // The pairs already joined, to reject self-loops and repeats: bridges as
  // (smaller island, larger island), sea routes and prerequisites in order.
  std::set<std::pair<int, int>> bridge_pairs;
  std::set<std::pair<int, int>> route_pairs;
  std::set<std::pair<int, int>> prerequisite_pairs;
};

// Thrown when a function declared in include/ returns a malformed result.
struct MalformedResult {
  std::string message;
};

// Splits `line` into the fields between single spaces. An empty field, caused
// by a leading, trailing, or double space, makes the line unparseable.
std::optional<std::vector<std::string_view>> split(std::string_view line) {
  std::vector<std::string_view> fields;
  std::size_t start = 0;
  while (true) {
    const std::size_t end = line.find(' ', start);
    const std::string_view field = line.substr(start, end - start);
    if (field.empty()) {
      return std::nullopt;
    }
    fields.push_back(field);
    if (end == std::string_view::npos) {
      return fields;
    }
    start = end + 1;
  }
}

// Converts `field` to an integer in [low, high], or returns std::nullopt if
// `field` is not exactly such an integer.
std::optional<std::int64_t> to_int(std::string_view field, std::int64_t low,
                                   std::int64_t high) {
  std::int64_t value = 0;
  const char *first = field.data();
  const char *last = field.data() + field.size();
  const auto [ptr, error] = std::from_chars(first, last, value);
  if (error != std::errc() || ptr != last || value < low || value > high) {
    return std::nullopt;
  }
  return value;
}

// Writes `values` to `out`, each preceded by a space.
template <typename T>
void write_list(std::ostream &out, const std::vector<T> &values) {
  for (const T &value : values) {
    out << ' ' << value;
  }
}

// Returns the path from the source to `target` described by `parent`, which
// must have `n` entries. Throws MalformedResult if the walk back from
// `target` leaves the range of vertices or does not reach `source` within n
// steps.
std::vector<int> path_to(const std::vector<int> &parent, int n, int source,
                         int target, const std::string &function) {
  std::vector<int> reversed{target};
  int v = target;
  while (v != source) {
    v = parent[v];
    if (v < 0 || v >= n || static_cast<int>(reversed.size()) > n) {
      throw MalformedResult{function + " returned a parent vector that does "
                                       "not lead back to the source"};
    }
    reversed.push_back(v);
  }
  return {reversed.rbegin(), reversed.rend()};
}

// Throws MalformedResult unless `paths` holds two vectors of size n.
void check_sizes(const ShortestPaths &paths, int n,
                 const std::string &function) {
  if (static_cast<int>(paths.dist.size()) != n ||
      static_cast<int>(paths.parent.size()) != n) {
    throw MalformedResult{function + " returned vectors of the wrong size"};
  }
}

// Throws MalformedResult unless every entry of `indices` lies in [0, n).
void check_indices(const std::vector<int> &indices, int n,
                   const std::string &function) {
  for (const int i : indices) {
    if (i < 0 || i >= n) {
      throw MalformedResult{function + " returned an index out of range"};
    }
  }
}

// Each bridge as two directed edges, for Dijkstra's algorithm.
std::vector<Edge> both_directions(const std::vector<Edge> &bridges) {
  std::vector<Edge> edges;
  edges.reserve(2 * bridges.size());
  for (const Edge &bridge : bridges) {
    edges.push_back(bridge);
    edges.push_back({bridge.to, bridge.from, bridge.weight});
  }
  return edges;
}

void query_regions(const World &world, std::ostream &out) {
  const int n = world.islands;
  const std::vector<int> label = region_labels(n, world.bridges);
  if (static_cast<int>(label.size()) != n) {
    throw MalformedResult{"region_labels returned a vector of the wrong size"};
  }
  check_indices(label, n, "region_labels");
  // Group the islands by label; a label must be the smallest island of its
  // region, and so must label itself.
  std::vector<std::vector<int>> members(n);
  for (int v = 0; v < n; ++v) {
    if (label[v] > v || label[label[v]] != label[v]) {
      throw MalformedResult{"region_labels returned an invalid label"};
    }
    members[label[v]].push_back(v);
  }
  int count = 0;
  for (int v = 0; v < n; ++v) {
    count += label[v] == v ? 1 : 0;
  }
  out << "regions " << count << '\n';
  for (int v = 0; v < n; ++v) {
    if (label[v] == v) {
      out << members[v][0];
      for (std::size_t i = 1; i < members[v].size(); ++i) {
        out << ' ' << members[v][i];
      }
      out << '\n';
    }
  }
}

void query_hops(const World &world, int s, int t, std::ostream &out) {
  const int hops = fewest_hops(world.islands, world.bridges, s, t);
  if (hops < -1 || hops >= world.islands) {
    throw MalformedResult{"fewest_hops returned an impossible number"};
  }
  out << "hops " << s << ' ' << t << ' ';
  if (hops == -1) {
    out << "unreachable\n";
  } else {
    out << hops << '\n';
  }
}

void query_maintain(const World &world, bool use_prim, std::ostream &out) {
  const SpanningForest forest = use_prim ? prim(world.islands, world.bridges)
                                         : kruskal(world.islands, world.bridges);
  out << "maintain " << forest.total_length << ' ' << forest.edges.size()
      << '\n';
  for (const Edge &edge : forest.edges) {
    out << edge.from << ' ' << edge.to << ' ' << edge.weight << '\n';
  }
}

void query_fastest(const World &world, int s, int t, std::ostream &out) {
  const int n = world.islands;
  const ShortestPaths paths = dijkstra(n, both_directions(world.bridges), s);
  check_sizes(paths, n, "dijkstra");
  out << "fastest " << s << ' ' << t << ' ';
  if (paths.dist[t] == kUnreachable) {
    out << "unreachable\n";
    return;
  }
  out << paths.dist[t];
  write_list(out, path_to(paths.parent, n, s, t, "dijkstra"));
  out << '\n';
}

void query_stamina(const World &world, int s, int t, std::ostream &out) {
  const int n = world.islands;
  const BellmanFordResult result = bellman_ford(n, world.routes, s);
  out << "stamina " << s << ' ' << t << ' ';
  if (!result.negative_cycle.empty()) {
    check_indices(result.negative_cycle, n, "bellman_ford");
    out << "exploit";
    write_list(out, result.negative_cycle);
    out << ' ' << result.negative_cycle.front() << '\n';
    return;
  }
  check_sizes(result.paths, n, "bellman_ford");
  if (result.paths.dist[t] == kUnreachable) {
    out << "unreachable\n";
    return;
  }
  out << result.paths.dist[t];
  write_list(out, path_to(result.paths.parent, n, s, t, "bellman_ford"));
  out << '\n';
}

void query_unlock(const World &world, int q, std::ostream &out) {
  const int k = static_cast<int>(world.duration.size());
  const std::optional<QuestSchedule> schedule =
      quest_schedule(world.duration, world.prerequisites);
  out << "unlock " << q << ' ';
  if (!schedule) {
    out << "cycle\n";
    return;
  }
  if (static_cast<int>(schedule->finish.size()) != k ||
      static_cast<int>(schedule->critical.size()) != k) {
    throw MalformedResult{"quest_schedule returned vectors of the wrong size"};
  }
  // The chain of critical prerequisites that ends at q, first quest first.
  std::vector<int> reversed{q};
  for (int p = schedule->critical[q]; p != -1; p = schedule->critical[p]) {
    if (p < -1 || p >= k || static_cast<int>(reversed.size()) >= k) {
      throw MalformedResult{"quest_schedule returned an invalid critical "
                            "vector"};
    }
    reversed.push_back(p);
  }
  out << schedule->finish[q];
  write_list(out, std::vector<int>(reversed.rbegin(), reversed.rend()));
  out << '\n';
}

void query_events(const World &world, bool weighted, std::ostream &out) {
  const int n = static_cast<int>(world.events.size());
  const std::vector<int> chosen =
      weighted ? best_events(world.events) : most_events(world.events);
  check_indices(chosen, n, weighted ? "best_events" : "most_events");
  if (weighted) {
    std::int64_t reward = 0;
    for (const int i : chosen) {
      reward += world.events[i].reward;
    }
    out << "best " << reward;
  } else {
    out << "most " << chosen.size();
  }
  write_list(out, chosen);
  out << '\n';
}

// Executes the command in `line` on `world` and writes its output to `out`.
// Returns false if the line cannot be parsed or is not valid for `world`.
bool execute(std::string_view line, World &world, std::ostream &out) {
  const auto fields = split(line);
  if (!fields) {
    return false;
  }
  const std::vector<std::string_view> &f = *fields;
  const std::string_view command = f[0];
  const std::size_t n_fields = f.size();

  if (command == "N" && n_fields == 3) {
    const auto islands = to_int(f[1], 1, kMaxCount);
    const auto quests = to_int(f[2], 0, kMaxCount);
    if (!islands || !quests) {
      return false;
    }
    world = World{};
    world.defined = true;
    world.islands = static_cast<int>(*islands);
    world.duration.assign(*quests, 0);
    return true;
  }
  if (!world.defined) {
    return false;
  }
  const int n = world.islands;
  const int k = static_cast<int>(world.duration.size());

  if ((command == "B" || command == "R") && n_fields == 4) {
    const bool bridge = command == "B";
    const auto u = to_int(f[1], 0, n - 1);
    const auto v = to_int(f[2], 0, n - 1);
    const auto w = to_int(f[3], bridge ? 0 : -kMaxWeight, kMaxWeight);
    if (!u || !v || !w || *u == *v) {
      return false;
    }
    const int a = static_cast<int>(*u);
    const int b = static_cast<int>(*v);
    // A bridge is undirected, so it is stored by its smaller island first.
    const bool added =
        bridge
            ? world.bridge_pairs.insert({std::min(a, b), std::max(a, b)}).second
            : world.route_pairs.insert({a, b}).second;
    if (!added) {
      return false;
    }
    (bridge ? world.bridges : world.routes).push_back({a, b, *w});
    return true;
  }
  if (command == "Q" && n_fields == 3) {
    const auto q = to_int(f[1], 0, k - 1);
    const auto hours = to_int(f[2], 0, kMaxWeight);
    if (!q || !hours) {
      return false;
    }
    world.duration[*q] = *hours;
    return true;
  }
  if (command == "P" && n_fields == 3) {
    const auto before = to_int(f[1], 0, k - 1);
    const auto after = to_int(f[2], 0, k - 1);
    if (!before || !after || *before == *after ||
        !world.prerequisite_pairs
             .insert({static_cast<int>(*before), static_cast<int>(*after)})
             .second) {
      return false;
    }
    world.prerequisites.push_back(
        {static_cast<int>(*before), static_cast<int>(*after)});
    return true;
  }
  if (command == "E" && n_fields == 4) {
    const auto start = to_int(f[1], 0, kMaxTime - 1);
    const auto finish = to_int(f[2], 1, kMaxTime);
    const auto reward = to_int(f[3], 1, kMaxWeight);
    if (!start || !finish || !reward || *start >= *finish) {
      return false;
    }
    world.events.push_back({*start, *finish, *reward});
    return true;
  }
  if ((command == "regions" || command == "most" || command == "best") &&
      n_fields == 1) {
    if (command == "regions") {
      query_regions(world, out);
    } else {
      query_events(world, command == "best", out);
    }
    return true;
  }
  if ((command == "hops" || command == "fastest" || command == "stamina") &&
      n_fields == 3) {
    const auto s = to_int(f[1], 0, n - 1);
    const auto t = to_int(f[2], 0, n - 1);
    if (!s || !t) {
      return false;
    }
    if (command == "hops") {
      query_hops(world, static_cast<int>(*s), static_cast<int>(*t), out);
    } else if (command == "fastest") {
      query_fastest(world, static_cast<int>(*s), static_cast<int>(*t), out);
    } else {
      query_stamina(world, static_cast<int>(*s), static_cast<int>(*t), out);
    }
    return true;
  }
  if (command == "maintain" && n_fields == 2 &&
      (f[1] == "kruskal" || f[1] == "prim")) {
    query_maintain(world, f[1] == "prim", out);
    return true;
  }
  if (command == "unlock" && n_fields == 2) {
    const auto q = to_int(f[1], 0, k - 1);
    if (!q) {
      return false;
    }
    query_unlock(world, static_cast<int>(*q), out);
    return true;
  }
  return false;
}

}  // namespace

int main(int argc, char *argv[]) {
  // Speeds up output for large command files.
  std::ios::sync_with_stdio(false);

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <command_file>\n";
    return EXIT_FAILURE;
  }
  std::ifstream in(argv[1]);
  if (!in) {
    std::cerr << "Cannot open " << argv[1] << '\n';
    return EXIT_FAILURE;
  }

  World world;
  std::string line;
  while (std::getline(in, line)) {
    // Command files edited on Windows may end each line with '\r'.
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty() || line.front() == '#') {
      continue;
    }
    try {
      if (!execute(line, world, std::cout)) {
        std::cerr << "Cannot parse line: " << line << '\n';
        return EXIT_FAILURE;
      }
    } catch (const MalformedResult &error) {
      std::cout.flush();
      std::cerr << "Malformed result for line: " << line << '\n'
                << error.message << '\n';
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
