#ifndef ROOT_MANAGER_SRC_REQUEST_TYPES_H_
#define ROOT_MANAGER_SRC_REQUEST_TYPES_H_

#include <string>
#include <set>
#include <variant>
#include <vector>
#include <unordered_map>
#include <map>

namespace rm {
struct RoutingSettings {
  int bus_wait_time;
  double bus_velocity;
};

struct GetBusRequest {
  int id;
  std::string bus;
};

struct GetStopRequest {
  int id;
  std::string stop;
};

struct GetRouteRequest {
  int id;
  std::string from;
  std::string to;
};

struct PostBusRequest {
  std::string bus;
  std::vector<std::string> stops;
};

struct Coords {
  double latitude, longitude;
};

struct RoadDistance {
  std::string destination;
  int distance;
};

struct PostStopRequest {
  std::string stop;
  Coords coords;
  // Pairs of stop_to and dist from stop to stop_to.
  std::map<std::string, int> stop_distances;
};

using PostRequest = std::variant<PostBusRequest, PostStopRequest>;
using GetRequest = std::variant<GetBusRequest, GetStopRequest, GetRouteRequest>;

struct StopInfo {
  using Dists = std::unordered_map<std::string, int>;
  // Distances to other stops.
  Dists dists;
  Coords coords;
  std::vector<std::string> buses;
};

struct BusInfo {
  std::vector<std::string> stops;
  int unique_stop_count;
  double distance;
  double curvature;
};

using StopDict = std::unordered_map<std::string, StopInfo>;
using BusDict = std::unordered_map<std::string, BusInfo>;
}

#endif // ROOT_MANAGER_SRC_REQUEST_TYPES_H_
