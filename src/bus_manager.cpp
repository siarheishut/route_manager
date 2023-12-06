#include "bus_manager.h"

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "distance_computer.h"
#include "transport_catalog.h"

using namespace rm::utils;

namespace {
int ComputeUniqueCount(std::vector<std::string> stops) {
  std::sort(begin(stops), end(stops));
  return std::distance(stops.begin(),
                       std::unique(begin(stops), end(stops)));
}
}

namespace rm {
BusManager::BusManager(std::shared_ptr<TransportCatalog> transport_catalog,
                       const RoutingSettings &routing_settings)
    : transport_catalog_(transport_catalog) {

  route_manager_ =
      std::make_unique<RouteManager>(transport_catalog_->Stops(),
                                     transport_catalog_->Buses(),
                                     routing_settings);
}

std::optional<BusResponse> BusManager::GetBusInfo(const std::string &bus) const {
  auto it = transport_catalog_->Buses().find(bus);
  if (it == transport_catalog_->Buses().end()) return std::nullopt;

  auto &b = it->second;
  return BusResponse{
      .stop_count = static_cast<int>(b.stops.size()),
      .unique_stop_count = b.unique_stop_count,
      .length = b.distance,
      .curvature = b.curvature,
  };
}

std::optional<StopResponse> BusManager::GetStopInfo(const std::string &stop) const {
  auto it = transport_catalog_->Stops().find(stop);
  if (it == transport_catalog_->Stops().end())
    return std::nullopt;

  return StopResponse{it->second.buses};
}

std::optional<RouteResponse> BusManager::GetRoute(const std::string &from,
                                                  const std::string &to) const {
  return route_manager_->FindRoute(from, to);
}
}
