// DO NOT MODIFY THIS FILE. Gradescope replaces it with the original before
// building your submission.
//
// Week 12: a greedy algorithm. Week 11: dynamic programming.

#ifndef EVENTS_H
#define EVENTS_H

#include <cstdint>
#include <vector>

// A timed event that runs from `start` up to, but not including, `finish`,
// with start < finish, and earns `reward` > 0. Two events are compatible if
// one finishes no later than the other starts.
struct Event {
  std::int64_t start;
  std::int64_t finish;
  std::int64_t reward;
};

// Returns the indices, in ascending order, of a largest set of pairwise
// compatible events. Use the greedy algorithm that repeatedly selects the
// compatible event that finishes first.
std::vector<int> most_events(const std::vector<Event> &events);

// Returns the indices, in ascending order, of a set of pairwise compatible
// events with the largest total reward. Use dynamic programming in
// O(n lg n) time for n events.
std::vector<int> best_events(const std::vector<Event> &events);

#endif  // EVENTS_H
