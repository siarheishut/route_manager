#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include "route_manager.h"
#include "request_types.h"

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <map>
#include <vector>

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
  static std::unique_ptr<BusManager> Create(std::vector<PostRequest> requests,
                                            const RoutingSettings &settings);

  std::optional<BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<StopResponse> GetStopInfo(const std::string &stop) const;

  std::optional<RouteResponse> FindRoute(const std::string &from,
                                         const std::string &to) const;
 private:
  BusManager(std::vector<PostRequest> requests,
             const RoutingSettings &settings);

  void AddStop(const std::string &stop, sphere::Coords coords,
               std::map<std::string, int> stops);

  void AddBus(std::string bus, std::vector<std::string> stops);
 private:

  StopDict stop_info_;
  BusDict bus_info_;
  std::unique_ptr<RouteManager> bus_manager_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
