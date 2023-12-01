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

namespace rm {
class BusManager {
 public:
  static std::unique_ptr<BusManager> Create(
      std::vector<utils::PostRequest> requests,
      const utils::RoutingSettings &routing_setting);

  std::optional<utils::BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<utils::StopResponse> GetStopInfo(const std::string &stop) const;

  std::optional<utils::RouteResponse> GetRoute(const std::string &from,
                                               const std::string &to) const;

 private:
  explicit BusManager(std::vector<utils::PostRequest> requests,
                      const utils::RoutingSettings &routing_settings);

  void AddStop(const std::string &stop, sphere::Coords coords,
               const std::map<std::string, int> &stops);

  void AddBus(std::string bus, std::vector<std::string> stops);

 private:
  utils::StopDict stop_info_;
  utils::BusDict bus_info_;
  std::unique_ptr<RouteManager> route_manager_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
