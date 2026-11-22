// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.

#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include <vector>

// A disjoint-set forest over the elements 0, ..., n - 1, with union by rank
// and path compression (Cormen et al., Section 19.3).
class DisjointSet {
 public:
  // Creates n singleton sets.
  explicit DisjointSet(int n);

  // Returns the representative of the set that contains x, compressing the
  // path from x to the root.
  int find(int x);

  // Merges the sets that contain x and y, attaching the root of lower rank
  // below the root of higher rank. Returns false, and changes nothing, if x
  // and y are already in the same set.
  bool unite(int x, int y);

  // Returns the number of disjoint sets.
  int count() const;

 private:
  std::vector<int> parent_;
  std::vector<int> rank_;
  int count_ = 0;
};

#endif  // DISJOINT_SET_H
