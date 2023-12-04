#ifndef ROOT_MANAGER_SRC_COMMON_H_
#define ROOT_MANAGER_SRC_COMMON_H_

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "request_types.h"

namespace rm::utils {
struct Route {
  std::vector<std::string_view> route;
  std::unordered_set<std::string_view> endpoints;
};

struct StopInfo {
  using Dists = std::unordered_map<std::string, int>;
  // Distances to other stops.
  Dists dists;
  sphere::Coords coords;
  std::vector<std::string> buses;
};

struct BusInfo {
  std::vector<std::string> stops;
  std::unordered_set<std::string> endpoints;
  int unique_stop_count;
  double distance;
  double curvature;
};

struct RouteInfo {
  struct RoadItem {
    std::string bus;
    double time;
    int start_idx;
    int span_count;
  };
  struct WaitItem {
    std::string stop;
    int time;
  };

  using Item = std::variant<RoadItem, WaitItem>;

  double time;
  std::vector<Item> items;
};

using StopDict = std::map<std::string, StopInfo>;
using BusDict = std::map<std::string, BusInfo>;

template<typename C, typename ...Args>
void Combine(C &container, Args... args) {
  (container.insert(begin(args), end(args)), ...);
}
}

#endif // ROOT_MANAGER_SRC_COMMON_H_
