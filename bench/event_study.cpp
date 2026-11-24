#include "events.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <numeric>
#include <random>
#include <stdexcept>

using Events = std::vector<Event>;
std::vector<int> greedy_selection(const Events& events, int rule) {
  std::vector<int> order(events.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int a, int b) {
    auto x = events[a], y = events[b];
    if (rule == 0 && x.reward != y.reward)
      return x.reward > y.reward;
    if (rule == 1 && x.finish != y.finish)
      return x.finish < y.finish;
    if (rule == 2) {

      auto left = x.reward * (y.finish - y.start), right = y.reward * (x.finish - x.start);
      if (left != right)
        return left > right;
    }
    return a < b;
  });

  std::map<std::int64_t, std::int64_t> chosen;
  std::vector<int> selected;
  for (int i : order) {
    auto e = events[i];
    auto next = chosen.lower_bound(e.start);
    if (next != chosen.end() && next->first < e.finish)
      continue;
    if (next != chosen.begin() && std::prev(next)->second > e.start)
      continue;
    chosen.emplace(e.start, e.finish);
    selected.push_back(i);
  }
  return selected;
}

std::int64_t score(const Events& events, const std::vector<int>& selected) {
  std::int64_t total = 0;
  for (int i : selected)
    total += events[i].reward;
  return total;
}

std::int64_t greedy(const Events& events, int rule) {
  return score(events, greedy_selection(events, rule));
}

std::int64_t optimal_reward(const Events& events) {
  return score(events, best_events(events));
}

void check_cases() {
  std::ofstream out("results/event-cases.txt");
  if (!out)
    throw std::runtime_error("run with results/ present");
  auto check = [&](const char* name, const Events& events, int rule, std::int64_t expected_greedy,
                   std::int64_t expected_best) {
    auto selected = greedy(events, rule);
    auto optimum = optimal_reward(events);
    if (selected != expected_greedy || optimum != expected_best)
      throw std::runtime_error("hand-case reward failed");
    out << name << ": greedy " << selected << ", optimum " << optimum << '\n';
  };
  check("Largest reward", {{0, 2, 6}, {2, 3, 4}, {0, 3, 9}}, 0, 9, 10);
  check("Earliest finish", {{0, 1, 1}, {0, 2, 2}}, 1, 1, 2);
  check("Density", {{0, 1, 3}, {0, 2, 5}}, 2, 3, 5);
  check("Proportional, longer first", {{0, 4, 20}, {3, 5, 10}}, 2, 20, 20);
  check("Proportional, shorter first", {{3, 5, 10}, {0, 4, 20}}, 2, 10, 20);
  check("Additive bonus, fixed order", {{0, 4, 20}, {3, 5, 15}}, 2, 15, 20);
  check("Compatible, longer first", {{0, 4, 20}, {5, 7, 10}}, 2, 30, 30);
  check("Compatible, shorter first", {{5, 7, 10}, {0, 4, 20}}, 2, 30, 30);
}

int main() {
  check_cases();
  std::ofstream out("results/events.csv");
  if (!out)
    throw std::runtime_error("run from the repository root with results/ present");
  out << "model,n,trial,optimal,reward,finish,density\n";

  std::mt19937 rng(6021);
  for (int n : {32, 128, 1000})
    for (int trial = 0; trial < 1000; ++trial) {
      Events independent, growing;
      for (int i = 0; i < n; ++i) {
        int start = std::uniform_int_distribution<int>(0, 2 * n - 1)(rng);
        int length = std::uniform_int_distribution<int>(1, 20)(rng);
        int reward = std::uniform_int_distribution<int>(1, 100)(rng);
        independent.push_back({start, start + length, reward});
        growing.push_back({start, start + length, 5 * length + (reward - 1) % 6});
      }
      for (bool scaled : {false, true}) {
        auto& events = scaled ? growing : independent;
        auto opt = optimal_reward(events);
        out << (scaled ? "growing" : "independent") << ',' << n << ',' << trial << ',' << opt;
        for (int rule = 0; rule < 3; ++rule) {
          auto value = greedy(events, rule);
          if (value > opt)
            throw std::runtime_error("greedy exceeded optimum");
          out << ',' << value;
        }
        out << '\n';
      }
    }
}
