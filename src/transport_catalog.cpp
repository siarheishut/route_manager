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

template<typename T>
int ComputeUniqueCount(std::vector<T> stops) {
  std::sort(begin(stops), end(stops));
  return std::distance(stops.begin(), std::unique(begin(stops), end(stops)));
}
}

namespace rm {
std::unique_ptr<TransportCatalog> TransportCatalog::Create(
    std::vector<PostRequest> requests) {
  // TODO: make sure that curvatures are more or equal to 1.
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

  std::set<std::string_view
  > route_stops, road_dists_stops, stop_requests_stops;
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

std::unique_ptr<TransportCatalog> TransportCatalog::Deserialize(
    const ::TransportCatalog::TransportDatabase &proto_database) {
  StopDict stops;
  BusDict buses;

  auto stop_count = proto_database.stops_size();

  for (auto &stop : proto_database.stops()) {
    auto lat = stop.coords().latitude(), lon = stop.coords().longitude();
    if (lat < kMinLatitude || lat > kMaxLatitude ||
        lon < kMinLongitude || lon > kMaxLongitude)
      return nullptr;
    stops[stop.stop_name()].coords = {.latitude = lat, .longitude = lon};
  }

  for (auto &bus : proto_database.buses()) {
    std::unordered_set<int> route_stops;
    if (bus.route_size() < 3 ||
        *bus.route().begin() != *(std::prev(bus.route().end())))
      return nullptr;
    for (auto idx : bus.route()) {
      if (idx >= stop_count)
        return nullptr;
      route_stops.insert(idx);
    }
    for (auto idx : bus.endpoints()) {
      if (route_stops.find(idx) == route_stops.end())
        return nullptr;
    }
    if (bus.curvature() < 1.0)
      return nullptr;

    auto &bus_name = bus.bus_name();
    auto &bus_info = buses[bus_name];
    bus_info = {
        .unique_stop_count = ComputeUniqueCount(
            std::vector<int>{bus.route().begin(), bus.route().end()}),
        .distance = static_cast<double>(bus.length()),
        .curvature = bus.curvature()};
    for (auto idx : bus.endpoints()) {
      bus_info.endpoints.insert(
          proto_database.stops().at(idx).stop_name());
    }
    for (auto idx : bus.route()) {
      auto &stop_name = proto_database.stops().at(idx).stop_name();
      bus_info.stops.push_back(stop_name);
      stops[stop_name].buses.push_back(bus_name);
    }
  }
  for (auto &[name, info] : stops) {
    std::sort(begin(info.buses), end(info.buses));
    info.buses.erase(std::unique(begin(info.buses), end(info.buses)),
                     end(info.buses));
  }

  if (buses.size() != proto_database.buses_size())
    return nullptr;
  if (stops.size() != proto_database.stops_size())
    return nullptr;

  return std::unique_ptr<TransportCatalog>(
      new TransportCatalog(std::move(stops), std::move(buses)));
}

::TransportCatalog::TransportDatabase TransportCatalog::Serialize() const {
  ::TransportCatalog::TransportDatabase proto_database;
  std::unordered_map<std::string_view, int> stop_to_idx;
  int idx = 0;
  for (auto &[stop, _] : stops_) {
    stop_to_idx[stop] = idx++;
  }

  for (auto &[stop, info] : stops_) {
    auto &proto_stop = *proto_database.add_stops();
    proto_stop.set_stop_name(stop);
    proto_stop.mutable_coords()->set_latitude(info.coords.latitude);
    proto_stop.mutable_coords()->set_longitude(info.coords.longitude);
  }
  for (auto &[bus, info] : buses_) {
    auto &proto_bus = *proto_database.add_buses();
    proto_bus.set_bus_name(bus);
    proto_bus.set_length(info.distance);
    proto_bus.set_curvature(info.curvature);
    for (auto &stop : info.stops) {
      proto_bus.add_route(stop_to_idx[stop]);
    }
    for (auto &stop : info.endpoints) {
      proto_bus.add_endpoints(stop_to_idx[stop]);
    }
  }
  return proto_database;
}

const utils::StopDict &TransportCatalog::Stops() const {
  return stops_;
}

const utils::BusDict &TransportCatalog::Buses() const {
  return buses_;
}

TransportCatalog::TransportCatalog(std::vector<PostRequest> requests) {
  // TODO: move all logic to factory method.
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
    bus_info.unique_stop_count =
        ComputeUniqueCount(std::vector<std::string_view>{
            bus_info.stops.begin(),
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

TransportCatalog::TransportCatalog(StopDict stops, BusDict buses)
    : stops_(std::move(stops)), buses_(std::move(buses)) {}

void TransportCatalog::AddStop(std::string stop, sphere::Coords coords,
                               const std::map<std::string, int> &stops) {
  auto [stop_it, _] = stops_.emplace(std::move(stop), StopInfo{});

  stop_it->second.coords = {coords.latitude, coords.longitude};
  // TODO: save road_distances in separate structure.
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
