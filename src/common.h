#ifndef ROOT_MANAGER_SRC_COMMON_H_
#define ROOT_MANAGER_SRC_COMMON_H_

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

#include "request_types.h"

namespace rm {
struct StopInfo {
  using Dists = std::unordered_map<std::string, int>;
  // Distances to other stops.
  Dists dists;
  sphere::Coords coords;
  std::vector<std::string> buses;
};

struct BusInfo {
  std::vector<std::string> stops;
  int unique_stop_count;
  double distance;
  double curvature;
};

struct RouteInfo {
  struct RoadItem {
    std::string bus;
    double time;
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

using Map = std::string;

using StopDict = std::unordered_map<std::string, StopInfo>;
using BusDict = std::unordered_map<std::string, BusInfo>;
}

#endif // ROOT_MANAGER_SRC_COMMON_H_
