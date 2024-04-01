#ifndef ROOT_MANAGER_SRC_ROUTE_MANAGER_H_
#define ROOT_MANAGER_SRC_ROUTE_MANAGER_H_

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include "router.h"

#include "common.h"
#include "graph.h"
#include "request_types.h"
#include "route_manager_utils.h"
#include "transport_catalog.h"
#include "transport_catalog.pb.h"

namespace rm {
class RouteManager {
 public:
  static std::unique_ptr<RouteManager> Deserialize(
      const ::TransportCatalog::TransportCatalog &proto_catalog,
      std::shared_ptr<TransportCatalog> catalog);

  RouteManager(const rm::utils::StopDict &stop_info,
               const rm::utils::BusDict &bus_info,
               const rm::utils::RoutingSettings &routing_settings);

  std::optional<utils::RouteInfo> FindRoute(const std::string &from,
                                            const std::string &to);

  ::TransportCatalog::Router Serialize(
      const ::TransportCatalog::TransportDatabase &proto_db) const;

 private:
  struct Params {
    std::unique_ptr<graph::Router<double>> router;
    graph::DirectedWeightedGraph<double> graph;
    rm::utils::RoutingSettings settings;
    std::vector<rm::route_manager::Edge> edges;
    std::vector<std::string> stop_names;
    std::unordered_map<std::string, size_t> stop_ids;
  };

  explicit RouteManager(Params config);

  void ReadStops(const rm::utils::StopDict &stop_dict);
  void ReadBuses(const rm::utils::BusDict &stop_dict,
                 const rm::utils::StopDict &bus_dict);

  std::unique_ptr<route_manager::Router> router_;
  route_manager::Graph graph_;
  rm::utils::RoutingSettings settings_;
  std::vector<route_manager::Edge> edges_;
  std::vector<std::string> stop_names_;
  std::unordered_map<std::string, size_t> stop_ids_;
};
}

#endif // ROOT_MANAGER_SRC_ROUTE_MANAGER_H_
