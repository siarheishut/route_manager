#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "request_types.h"
#include "response_types.h"
#include "route_manager.h"
#include "transport_catalog.h"

namespace rm {
class BusManager {
 public:
  explicit BusManager(std::shared_ptr<TransportCatalog> transport_catalog,
                      const utils::RoutingSettings &routing_setting);

  std::optional<utils::BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<utils::StopResponse> GetStopInfo(const std::string &stop) const;

  std::optional<utils::RouteResponse> GetRoute(const std::string &from,
                                               const std::string &to) const;

 private:
  std::shared_ptr<TransportCatalog> transport_catalog_;
  std::unique_ptr<route_manager::RouteManager> route_manager_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
