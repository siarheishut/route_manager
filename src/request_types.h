#ifndef ROOT_MANAGER_SRC_REQUEST_TYPES_H_
#define ROOT_MANAGER_SRC_REQUEST_TYPES_H_

#include "sphere.h"

#include <string>
#include <variant>
#include <vector>
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

struct PostStopRequest {
  std::string stop;
  sphere::Coords coords;
  // Pairs of stop_to and dist from stop to stop_to.
  std::map<std::string, int> stop_distances;
};

using PostRequest = std::variant<PostBusRequest, PostStopRequest>;
using GetRequest = std::variant<GetBusRequest, GetStopRequest, GetRouteRequest>;
}

#endif // ROOT_MANAGER_SRC_REQUEST_TYPES_H_
