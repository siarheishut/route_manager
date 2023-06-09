#ifndef ROOT_MANAGER_SRC_ROUTE_MANAGER_H_
#define ROOT_MANAGER_SRC_ROUTE_MANAGER_H_

#include "src/request_types.h"
#include "router.h"
#include "common.h"

#include <optional>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>
#include <utility>
#include <string>

namespace rm {
class RouteManager {
 private:
  using Graph = graph::DirectedWeightedGraph<double>;
  using Router = graph::Router<double>;

 public:
  RouteManager(const rm::StopDict &stop_info,
               const rm::BusDict &bus_info,
               const rm::RoutingSettings &routing_settings);

  std::optional<RouteInfo> FindRoute(const std::string &from,
                                     const std::string &to);

 private:
  void ReadStops(const rm::StopDict &stop_dict);
  void ReadBuses(const rm::BusDict &stop_dict, const rm::StopDict &bus_dict);

  using Vertex = std::string;

  struct StopIds {
    graph::VertexId arrive;
    graph::VertexId depart;
  };

  struct RoadEdge {
    std::string bus;
    int span_count;
  };
  struct WaitEdge {};

  using Edge = std::variant<RoadEdge, WaitEdge>;

  std::unique_ptr<Router> router_;
  Graph graph_;
  rm::RoutingSettings settings_;
  std::vector<Edge> edges_;
  std::vector<Vertex> vertices_;
  std::unordered_map<std::string, StopIds> stop_ids_;
};
}

#endif // ROOT_MANAGER_SRC_ROUTE_MANAGER_H_
