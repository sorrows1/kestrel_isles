#include "disjoint_set.h"

#include <algorithm>
#include <numeric>

DisjointSet::DisjointSet(int n) : parent_(n), rank_(n), count_(n) {
  std::iota(parent_.begin(), parent_.end(), 0);
}

int DisjointSet::find(int x) {
  int root = x;
  while (parent_[root] != root)
    root = parent_[root];
  while (x != root) {
    int next = parent_[x];
    parent_[x] = root;
    x = next;
  }
  return root;
}

bool DisjointSet::unite(int x, int y) {
  int a = find(x), b = find(y);
  if (a == b)
    return false;
  if (rank_[a] < rank_[b])
    std::swap(a, b);
  parent_[b] = a;
  if (rank_[a] == rank_[b])
    ++rank_[a];
  --count_;
  return true;
}

int DisjointSet::count() const {
  return count_;
}
