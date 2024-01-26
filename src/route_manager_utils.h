#ifndef ROOT_MANAGER_SRC_ROUTE_MANAGER_UTILS_H_
#define ROOT_MANAGER_SRC_ROUTE_MANAGER_UTILS_H_

#include <string>
#include <tuple>

#include "graph.h"
#include "router.h"

namespace rm::route_manager {
using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;

struct RoadEdge {
  std::string bus;
  int start_idx;
  int span_count;
};
struct WaitEdge {};

using Edge = std::variant<RoadEdge, WaitEdge>;
}

#endif // ROOT_MANAGER_SRC_ROUTE_MANAGER_UTILS_H_
