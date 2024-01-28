#include "route_manager.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "common.h"
#include "distance_computer.h"
#include "request_types.h"
#include "route_manager_utils.h"

using namespace rm::utils;

namespace {
using namespace rm::route_manager;
bool IsArrive(size_t vertex) {
  return (vertex % 2 == 0);
}

bool IsDepart(size_t vertex) {
  return (vertex % 2 == 1);
}

size_t ToStopId(size_t vertex) {
  return vertex / 2;
}

size_t ToArrive(size_t stop_id) {
  return stop_id * 2;
}

size_t ToDepart(size_t stop_id) {
  return stop_id * 2 + 1;
}
}

namespace rm::route_manager {
RouteManager::RouteManager(const StopDict &stop_info,
                           const BusDict &bus_info,
                           const RoutingSettings &routing_settings)
    : settings_(routing_settings),
      graph_(stop_info.size() * 2),
      stop_names_(stop_info.size()) {
  ReadStops(stop_info);
  ReadBuses(bus_info, stop_info);
  router_ = std::make_unique<Router>(graph_);
}

void RouteManager::ReadStops(const StopDict &stop_dict) {
  size_t stop_id = 0;
  for (auto &[stop, _] : stop_dict) {
    stop_ids_[stop] = stop_id;
    stop_names_[stop_id] = stop;
    graph_.AddEdge({
                       .from = ToArrive(stop_id),
                       .to = ToDepart(stop_id),
                       .weight = static_cast<double>(settings_.bus_wait_time)});
    edges_.emplace_back(WaitEdge{});
    stop_id++;
  }
}

void RouteManager::ReadBuses(const BusDict &bus_dict,
                             const StopDict &stop_dict) {
  for (auto &[bus, bus_info] : bus_dict) {
    auto &route = bus_info.stops;
    int stop_count = route.size();
    for (int from = 0; from + 1 < stop_count; ++from) {
      const auto depart = ToDepart(stop_ids_[route[from]]);
      double distance = 0.0;
      for (int to = from + 1; to < stop_count; ++to) {
        distance += ComputeRoadDistance({route[to - 1], route[to]}, stop_dict);
        edges_.emplace_back(RoadEdge{
            .bus = bus,
            .start_idx = from,
            .span_count = to - from});
        const auto arrive = ToArrive(stop_ids_[route[to]]);
        graph_.AddEdge(
            {
                .from = depart,
                .to = arrive,
                .weight = distance / (settings_.bus_velocity * 1000 / 60)
            });
      }
    }
  }
}

std::optional<RouteInfo> RouteManager::FindRoute(const std::string &from,
                                                 const std::string &to) {
  auto it_from = stop_ids_.find(from);
  auto it_to = stop_ids_.find(to);
  if (it_from == stop_ids_.end() || it_to == stop_ids_.end())
    return std::nullopt;

  auto vertex_from = ToArrive(it_from->second);
  auto vertex_to = ToArrive(it_to->second);
  auto route = router_->BuildRoute(vertex_from, vertex_to);
  if (!route) return std::nullopt;

  RouteInfo route_info;
  auto route_id = route->id;
  route_info.time = route->weight;
  for (int i = 0; i < route->edge_count; ++i) {
    auto edge_id = router_->GetRouteEdge(route_id, i);
    auto &edge = graph_.GetEdge(edge_id);

    if (auto ptr_r = std::get_if<RoadEdge>(&edges_[edge_id])) {
      route_info.items.emplace_back(RouteInfo::RoadItem{
          .bus = ptr_r->bus,
          .time = edge.weight,
          .start_idx = ptr_r->start_idx,
          .span_count = ptr_r->span_count});
    } else if (std::holds_alternative<WaitEdge>(edges_[edge_id])) {
      route_info.items.emplace_back(RouteInfo::WaitItem{
          .stop = stop_names_[ToStopId(edge.from)],
          .time = static_cast<int>(edge.weight)});
    }
  }

  router_->ReleaseRoute(route->id);

  return route_info;
}
}
