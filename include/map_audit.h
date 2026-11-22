// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.
//
// Week 08: breadth-first and depth-first search on the bridge network.

#ifndef MAP_AUDIT_H
#define MAP_AUDIT_H

#include <vector>

#include "graph.h"

// Returns a vector `label` of size n in which label[v] is the smallest island
// in the region of island v, where a region is a connected component of the
// undirected graph with islands 0, ..., n - 1 and the given bridges. Bridge
// lengths are ignored. Use breadth-first or depth-first search.
std::vector<int> region_labels(int n, const std::vector<Edge> &bridges);

// Returns the fewest bridges on any path from island s to island t, 0 if
// s == t, or -1 if t cannot be reached from s. Bridge lengths are ignored.
// Use breadth-first search.
int fewest_hops(int n, const std::vector<Edge> &bridges, int s, int t);

#endif  // MAP_AUDIT_H
