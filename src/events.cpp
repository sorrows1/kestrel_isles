#include "events.h"

#include <algorithm>
#include <numeric>
#include <tuple>

namespace {
std::vector<int> finish_order(const std::vector<Event>& events) {
  std::vector<int> order(events.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int a, int b) {
    return std::tie(events[a].finish, a) < std::tie(events[b].finish, b);
  });
  return order;
}
}

std::vector<int> most_events(const std::vector<Event>& events) {
  std::vector<int> selected;
  std::int64_t free_at = 0;

  for (int i : finish_order(events)) {
    if (events[i].start >= free_at) {
      selected.push_back(i);
      free_at = events[i].finish;
    }
  }

  std::sort(selected.begin(), selected.end());
  return selected;
}

std::vector<int> best_events(const std::vector<Event>& events) {
  auto order = finish_order(events);
  int n = static_cast<int>(order.size());
  std::vector<std::int64_t> ends, best(n + 1);
  std::vector<int> compatible(n + 1);

  for (int j = 1; j <= n; ++j) {
    const auto& e = events[order[j - 1]];
    compatible[j] = std::upper_bound(ends.begin(), ends.end(), e.start) - ends.begin();
    best[j] = std::max(best[j - 1], e.reward + best[compatible[j]]);
    ends.push_back(e.finish);
  }

  std::vector<int> selected;

  for (int j = n; j > 0;) {
    if (best[j] == best[j - 1]) {
      --j;
    } else {
      selected.push_back(order[j - 1]);
      j = compatible[j];
    }
  }

  std::sort(selected.begin(), selected.end());
  return selected;
}
