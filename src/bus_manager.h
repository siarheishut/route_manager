#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "svg/common.h"

#include "common.h"
#include "coords_converter.h"
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

using MapResponse = Map;

class BusManager {
 public:
  static std::unique_ptr<BusManager> Create(std::vector<PostRequest> requests,
                                            RoutingSettings routing_setting,
                                            RenderingSettings rendering_settings);

  std::optional<BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<StopResponse> GetStopInfo(const std::string &stop) const;

  std::optional<RouteResponse> GetRoute(const std::string &from,
                                        const std::string &to) const;

  MapResponse GetMap() const;

 private:
  explicit BusManager(std::vector<PostRequest> requests,
                      RoutingSettings routing_settings,
                      RenderingSettings rendering_settings,
                      CoordsConverter::Config coords_converter);

  void AddStop(const std::string &stop, sphere::Coords coords,
               const std::map<std::string, int> &stops);

  void AddBus(std::string bus, std::vector<std::string> stops);

 private:
  void RenderMap(std::map<std::string_view,
                          std::vector<std::string_view>> routes,
                 std::map<std::string_view, svg::Point> stops,
                 RenderingSettings settings);

 private:
  CoordsConverter converter_;
  StopDict stop_info_;
  BusDict bus_info_;
  std::unique_ptr<RouteManager> route_manager_;
  Map map_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
