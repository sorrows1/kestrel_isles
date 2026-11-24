#include "quests.h"
#include "packed_lists.h"

#include <numeric>
#include <queue>

std::optional<QuestSchedule> quest_schedule(const std::vector<std::int64_t>& duration,
                                            const std::vector<Prerequisite>& prerequisites) {
  int n = static_cast<int>(duration.size());
  PackedLists<int> next(n);
  std::vector<int> indegree(n);
  for (auto [u, v] : prerequisites) {
    ++next.offset[u + 1];
    ++indegree[v];
  }

  std::partial_sum(next.offset.begin(), next.offset.end(), next.offset.begin());
  next.items.resize(prerequisites.size());
  {
    auto cursor = next.offset;
    for (auto [u, v] : prerequisites)
      next.items[cursor[u]++] = v;
  }

  std::queue<int> ready;
  for (int u = 0; u < n; ++u)
    if (indegree[u] == 0)
      ready.push(u);
  QuestSchedule schedule{duration, std::vector<int>(n, -1)};
  int processed = 0;

  while (!ready.empty()) {
    int u = ready.front();
    ready.pop();
    ++processed;

    for (int v : next[u]) {
      auto finish = schedule.finish[u] + duration[v];

      if (schedule.critical[v] == -1 || finish > schedule.finish[v]) {
        schedule.finish[v] = finish;
        schedule.critical[v] = u;
      }
      if (--indegree[v] == 0)
        ready.push(v);
    }
  }

  if (processed != n)
    return std::nullopt;
  return schedule;
}
