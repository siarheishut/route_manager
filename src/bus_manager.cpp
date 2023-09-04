#include "bus_manager.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace {
// Used the solution from https://stackoverflow.com/a/70429229.
// Uses Haversine formula https://en.wikipedia.org/wiki/Haversine_formula
// with asin expressed with atan2 for better accuracy.
double CalculateDistance(rm::Coords lhs, rm::Coords rhs) {
  constexpr double earth_radius_m = 6371.0 * 1000.0;

  const double d_lat = rhs.latitude - lhs.latitude;
  const double d_long = rhs.longitude - lhs.longitude;

  double a =
      std::sin(d_lat / 2) * std::sin(d_lat / 2) + std::sin(d_long / 2) *
          std::sin(d_long / 2) *
          std::cos(lhs.latitude) *
          std::cos(rhs.latitude);
  double c = earth_radius_m * 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

  return c;
}

int ComputeUniqueCount(std::vector<std::string> stops) {
  std::sort(begin(stops), end(stops));
  return std::distance(stops.begin(),
                       std::unique(begin(stops), end(stops)));
}
}

namespace rm {
std::unique_ptr<BusManager> BusManager::Create(std::vector<PostRequest> requests) {
  std::set<std::string_view> route_stops, road_dists_stops, stop_requests_stops;
  std::set<std::string_view> buses;
  for (auto &req : requests) {
    if (auto ptr_b = std::get_if<PostBusRequest>(&req)) {
      route_stops.insert(ptr_b->stops.begin(), ptr_b->stops.end());
      if (!buses.insert(ptr_b->bus).second)
        return nullptr;
    } else if (auto ptr_s = std::get_if<PostStopRequest>(&req)) {
      for (auto &[dst_stop, dist] : ptr_s->stop_distances) {
        road_dists_stops.insert(dst_stop);
      }
      if (!stop_requests_stops.insert(ptr_s->stop).second)
        return nullptr;
    }
  }
  if (!std::includes(begin(stop_requests_stops), end(stop_requests_stops),
                     begin(route_stops), end(route_stops)))
    return nullptr;
  if (!std::includes(begin(stop_requests_stops), end(stop_requests_stops),
                     begin(road_dists_stops), end(road_dists_stops)))
    return nullptr;

  return std::unique_ptr<BusManager>(new BusManager(std::move(requests)));
}

BusManager::BusManager(std::vector<PostRequest> requests) {
  for (auto &request : requests) {
    if (std::holds_alternative<PostBusRequest>(request)) {
      auto &bus = std::get<PostBusRequest>(request);
      AddBus(std::move(bus.bus), std::move(bus.stops));
    } else if (std::holds_alternative<PostStopRequest>(request)) {
      auto &stop = std::get<PostStopRequest>(request);
      AddStop(stop.stop,
              stop.coords,
              stop.stop_distances);
    }
  }

  for (auto &[bus, bus_info] : bus_info_) {
    double geo_dist = ComputeGeoDistance(bus_info.stops);
    bus_info.distance = ComputeRoadDistance(bus_info.stops);
    bus_info.unique_stop_count = ComputeUniqueCount(bus_info.stops);
    bus_info.curvature = bus_info.distance / geo_dist;
  }
  for (auto &[_, info] : stop_info_) {
    auto &buses = info.buses;
    std::sort(buses.begin(), buses.end());
    buses.erase(std::unique(buses.begin(), buses.end()),
                buses.end());
  }
}

void BusManager::AddStop(const std::string &stop, Coords coords,
                         const std::map<std::string, int> &stops) {
  constexpr double k = 3.1415926535 / 180;
  auto &info = stop_info_[stop];
  info.coords = {coords.latitude * k, coords.longitude * k};
  for (auto &[stop_to, dist] : stops) {
    auto &stop_to_dists = stop_info_[stop_to].dists;
    if (auto it = stop_to_dists.find(stop); it == stop_to_dists.end()) {
      stop_to_dists[stop] = dist;
    }
    info.dists[stop_to] = dist;
  }
}

void BusManager::AddBus(std::string bus,
                        std::vector<std::string> stops) {
  for (auto &stop : stops)
    stop_info_[stop].buses.push_back(bus);

  BusInfo bus_info;
  bus_info.stops = std::move(stops);
  bus_info_[std::move(bus)] = bus_info;
}

std::optional<BusResponse> BusManager::GetBusInfo(const std::string &bus) const {
  auto it = bus_info_.find(bus);
  if (it == bus_info_.end()) return std::nullopt;

  auto &b = it->second;
  return BusResponse{
      .stop_count = static_cast<int>(b.stops.size()),
      .unique_stop_count = b.unique_stop_count,
      .length = b.distance,
      .curvature = b.curvature,
  };
}

std::optional<StopResponse> BusManager::GetStopInfo(const std::string &stop) const {
  auto it = stop_info_.find(stop);
  if (it == stop_info_.end())
    return std::nullopt;

  return StopResponse{it->second.buses};
}

double BusManager::ComputeGeoDistance(const std::vector<std::string> &stops) const {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    distance += CalculateDistance(stop_info_.at(stops[i - 1]).coords,
                                  stop_info_.at(stops[i]).coords);
  }
  return distance;
}

double BusManager::ComputeRoadDistance(const std::vector<std::string> &stops) const {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    auto &dists = stop_info_.at(stops[i - 1]).dists;

    if (auto found = dists.find(stops[i]); found != dists.end())
      distance += found->second;
    else
      distance += CalculateDistance(stop_info_.at(stops[i - 1]).coords,
                                    stop_info_.at(stops[i]).coords);
  }
  return distance;
}
}
