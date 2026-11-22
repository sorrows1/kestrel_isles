// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.

#ifndef GRAPH_H
#define GRAPH_H

#include <cstdint>
#include <limits>

// An edge from `from` to `to` with the given weight. Islands and quests are
// numbered from 0. A bridge is undirected and appears once in a list of
// bridges; a sea route is directed. There are no self-loops and no parallel
// edges: at most one bridge joins two islands, and at most one sea route
// leads from one island to another.
struct Edge {
  int from;
  int to;
  std::int64_t weight;
};

// The distance of a vertex that cannot be reached from the source.
inline constexpr std::int64_t kUnreachable =
    std::numeric_limits<std::int64_t>::max();

#endif  // GRAPH_H
