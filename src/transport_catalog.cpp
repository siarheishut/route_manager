#include "transport_catalog.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include "distance_computer.h"
#include "request_types.h"
#include "sphere.h"

using namespace rm::utils;

namespace {
// Latitude value range is [-90, 90];
// Longitude value range is [-180, 180].
// We can read about it in the following article:
// https://www.techtarget.com/whatis/definition/latitude-and-longitude.
const double kMinLatitude = -90.0, kMaxLatitude = 90.0,
    kMinLongitude = -180.0, kMaxLongitude = 180.0;

int ComputeUniqueCount(std::vector<std::string_view> stops) {
  std::sort(begin(stops), end(stops));
  return std::distance(stops.begin(),
                       std::unique(begin(stops), end(stops)));
}
}

namespace rm {
std::unique_ptr<TransportCatalog> TransportCatalog::Create(
    std::vector<PostRequest> requests) {
  for (auto &req : requests) {
    if (auto ptr_s = std::get_if<PostStopRequest>(&req)) {
      auto lat = ptr_s->coords.latitude, lon = ptr_s->coords.longitude;
      if (lat < kMinLatitude || lat > kMaxLatitude ||
          lon < kMinLongitude || lon > kMaxLongitude)
        return nullptr;
    } else if (auto ptr_b = std::get_if<PostBusRequest>(&req)) {
      auto stops = ptr_b->stops;
      if (stops.size() < 3 || stops.front() != stops.back())
        return nullptr;
      std::unordered_set<std::string_view> route_stops(stops.begin(),
                                                       stops.end());
      for (auto &endpoint : ptr_b->endpoints) {
        if (route_stops.find(endpoint) == route_stops.end())
          return nullptr;
      }
    }
  }

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

  return std::unique_ptr<TransportCatalog>(
      new TransportCatalog(std::move(requests)));
}

const utils::StopDict &TransportCatalog::Stops() const {
  return stops_;
}

const utils::BusDict &TransportCatalog::Buses() const {
  return buses_;
}

TransportCatalog::TransportCatalog(std::vector<PostRequest> requests) {
  for (auto &request : requests) {
    if (std::holds_alternative<PostBusRequest>(request)) {
      auto &bus = std::get<PostBusRequest>(request);
      AddBus(std::move(bus.bus), std::move(bus.stops),
             std::move(bus.endpoints));
    } else if (std::holds_alternative<PostStopRequest>(request)) {
      auto &stop = std::get<PostStopRequest>(request);
      AddStop(std::move(stop.stop),
              stop.coords,
              stop.stop_distances);
    }
  }

  for (auto &[bus, bus_info] : buses_) {
    double geo_dist = ComputeGeoDistance(bus_info.stops, stops_);
    bus_info.distance = ComputeRoadDistance(bus_info.stops, stops_);
    bus_info.unique_stop_count = ComputeUniqueCount({bus_info.stops.begin(),
                                                     bus_info.stops.end()});
    bus_info.curvature = bus_info.distance / geo_dist;
  }
  for (auto &[_, info] : stops_) {
    auto &buses = info.buses;
    std::sort(buses.begin(), buses.end());
    buses.erase(std::unique(buses.begin(), buses.end()),
                buses.end());
  }
}

void TransportCatalog::AddStop(std::string stop, sphere::Coords coords,
                               const std::map<std::string, int> &stops) {
  auto [stop_it, _] = stops_.emplace(std::move(stop), StopInfo{});

  stop_it->second.coords = {coords.latitude, coords.longitude};
  for (auto &[stop_to, dist] : stops) {
    auto &stop_to_dists = stops_[stop_to].dists;
    if (auto it = stop_to_dists.find(stop_it->first);
        it == stop_to_dists.end()) {
      stop_to_dists[stop_it->first] = dist;
    }
    stop_it->second.dists[stop_to] = dist;
  }
}

void TransportCatalog::AddBus(std::string bus,
                              std::vector<std::string> stops,
                              std::unordered_set<std::string> endpoints) {
  auto [bus_it, _] = buses_.emplace(std::move(bus), BusInfo{});

  for (auto &stop : stops)
    stops_[stop].buses.push_back(bus_it->first);

  bus_it->second.stops = {std::make_move_iterator(stops.begin()),
                          std::make_move_iterator(stops.end())};

  bus_it->second.endpoints = std::move(endpoints);
}
}
