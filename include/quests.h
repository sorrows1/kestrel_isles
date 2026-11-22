// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.
//
// Week 11: dynamic programming on a DAG.

#ifndef QUESTS_H
#define QUESTS_H

#include <cstdint>
#include <optional>
#include <vector>

// Quest `before` must be finished before quest `after` can start.
struct Prerequisite {
  int before;
  int after;
};

// The earliest finishing times of all quests when any number of quests can
// run at the same time. Both vectors have one entry per quest.
struct QuestSchedule {
  // finish[q] is duration[q] plus the largest finishing time of a
  // prerequisite of q, or plus 0 if q has no prerequisites.
  std::vector<std::int64_t> finish;
  // critical[q] is a prerequisite p of q with the largest finish[p], or -1
  // if q has no prerequisites.
  std::vector<int> critical;
};

// Returns the earliest finishing times of the quests 0, ..., k - 1, where
// k = duration.size(), or std::nullopt if the prerequisites contain a cycle.
// Use dynamic programming over a topological order.
std::optional<QuestSchedule> quest_schedule(
    const std::vector<std::int64_t> &duration,
    const std::vector<Prerequisite> &prerequisites);

#endif  // QUESTS_H
