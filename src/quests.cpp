#include "quests.h"

#include <queue>

std::optional<QuestSchedule> quest_schedule(const std::vector<std::int64_t>& duration,
                                            const std::vector<Prerequisite>& prerequisites) {
  int n = static_cast<int>(duration.size());
  std::vector<std::vector<int>> next(n);
  std::vector<int> indegree(n);
  for (auto [u, v] : prerequisites) {
    next[u].push_back(v);
    ++indegree[v];
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
      if (finish > schedule.finish[v]) {
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
