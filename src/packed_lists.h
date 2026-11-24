#pragma once

#include <cstddef>
#include <span>
#include <vector>

template <typename Entry> struct PackedLists {
  explicit PackedLists(int n) : offset(static_cast<std::size_t>(n) + 1) {
  }

  std::span<const Entry> operator[](int v) const {
    return std::span<const Entry>(items).subspan(offset[v], offset[v + 1] - offset[v]);
  }

  std::vector<std::size_t> offset;
  std::vector<Entry> items;
};
