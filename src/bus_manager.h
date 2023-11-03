#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "request_types.h"
#include "route_manager.h"

namespace rm {
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

class BusManager {
 public:
  static std::unique_ptr<BusManager> Create(
      std::vector<PostRequest> requests,
      const RoutingSettings &routing_setting);

  std::optional<BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<StopResponse> GetStopInfo(const std::string &stop) const;

  std::optional<RouteResponse> GetRoute(const std::string &from,
                                        const std::string &to) const;

 private:
  explicit BusManager(std::vector<PostRequest> requests,
                      const RoutingSettings &routing_settings);

  void AddStop(const std::string &stop, sphere::Coords coords,
               const std::map<std::string, int> &stops);

  void AddBus(std::string bus, std::vector<std::string> stops);

 private:
  StopDict stop_info_;
  BusDict bus_info_;
  std::unique_ptr<RouteManager> route_manager_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
