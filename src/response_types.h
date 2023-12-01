#ifndef ROOT_MANAGER_ROUTE_MANAGER_SRC_RESPONSE_TYPES_H_
#define ROOT_MANAGER_ROUTE_MANAGER_SRC_RESPONSE_TYPES_H_

#include <string>
#include <vector>

#include "common.h"

namespace rm::utils {
struct BusResponse {
  int stop_count;
  int unique_stop_count;
  double length;
  double curvature;
};

struct StopResponse {
  std::vector<std::string> buses;
};

using RouteResponse = RouteInfo;

struct MapResponse {
  std::string map;
};
}

#endif // ROOT_MANAGER_ROUTE_MANAGER_SRC_RESPONSE_TYPES_H_
