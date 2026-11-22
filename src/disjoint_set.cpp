// Your disjoint-set forest. Every member function below is a stub that
// compiles but does nothing useful yet; replace each one with your
// implementation.

#include "disjoint_set.h"

DisjointSet::DisjointSet(int /*n*/) {
  // TODO: Make every element the root of its own tree, with rank 0.

  // This line only silences a compiler warning about an unused member.
  // Delete it once your code uses rank_.
  (void)rank_;
}

int DisjointSet::find(int x) {
  return x;  // TODO
}

bool DisjointSet::unite(int /*x*/, int /*y*/) {
  return false;  // TODO
}

int DisjointSet::count() const {
  return count_;
}
