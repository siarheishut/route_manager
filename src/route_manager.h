#ifndef ROOT_MANAGER_SRC_ROUTE_MANAGER_H_
#define ROOT_MANAGER_SRC_ROUTE_MANAGER_H_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "router.h"

#include "common.h"
#include "request_types.h"
#include "route_manager_utils.h"

namespace rm::route_manager {
class RouteManager {
 public:
  RouteManager(const rm::utils::StopDict &stop_info,
               const rm::utils::BusDict &bus_info,
               const rm::utils::RoutingSettings &routing_settings);

  std::optional<utils::RouteInfo> FindRoute(const std::string &from,
                                            const std::string &to);

 private:
  void ReadStops(const rm::utils::StopDict &stop_dict);
  void ReadBuses(const rm::utils::BusDict &stop_dict,
                 const rm::utils::StopDict &bus_dict);

  std::unique_ptr<Router> router_;
  Graph graph_;
  rm::utils::RoutingSettings settings_;
  std::vector<Edge> edges_;
  std::vector<std::string> stop_names_;
  std::unordered_map<std::string, size_t> stop_ids_;
};
}

#endif // ROOT_MANAGER_SRC_ROUTE_MANAGER_H_
