#ifndef ROOT_MANAGER_SRC_BUS_CATALOG_BUS_CATALOG_H_
#define ROOT_MANAGER_SRC_BUS_CATALOG_BUS_CATALOG_H_

#include "src/request_types.h"
#include "router.h"

#include <optional>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>
#include <utility>
#include <string>

namespace rm {
double ComputeGeoDistance(const std::vector<rm::Coords> &stops);
double ComputeRoadDistance(const std::vector<std::string> &stops,
                           const StopDict &dict);
class RouteBase {
 private:
  using Graph = graph::DirectedWeightedGraph<double>;
  using Router = graph::Router<double>;
 public:
  RouteBase(const rm::StopDict &stop_info,
            const rm::BusDict &bus_info,
            const rm::RoutingSettings &routing_settings);

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

    double time;
    using Item = std::variant<RoadItem, WaitItem>;
    std::vector<Item> items;
  };

  std::optional<RouteInfo> FindRoute(const std::string &from,
                                     const std::string &to);

 private:
  void ReadStops(const rm::StopDict &stop_dict);
  void ReadBuses(const rm::BusDict &stop_dict, const rm::StopDict &bus_dict);
 private:
  struct Vertex {
    std::string stop;
  };
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
  std::vector<Vertex> vertices_;
  std::vector<Edge> edges_;
  std::unordered_map<std::string, StopIds> stop_to_ids_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_CATALOG_BUS_CATALOG_H_
