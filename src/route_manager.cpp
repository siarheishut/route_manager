#include "route_manager.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "common.h"
#include "distance_computer.h"
#include "request_types.h"

namespace {
std::vector<std::string> Route(const rm::BusInfo &bus_info) {
  auto route = bus_info.stops;
  if (!bus_info.is_roundtrip) {
    for (int i = static_cast<int>(route.size()) - 2; i >= 0; --i) {
      route.push_back(route[i]);
    }
  }
  return route;
}
}

namespace rm {
RouteManager::RouteManager(const rm::StopDict &stop_info,
                           const rm::BusDict &bus_info,
                           const rm::RoutingSettings &routing_settings)
    : settings_(routing_settings),
      graph_(stop_info.size() * 2),
      vertices_(stop_info.size() * 2) {
  ReadStops(stop_info);
  ReadBuses(bus_info, stop_info);
  router_ = std::make_unique<Router>(graph_);
}

void RouteManager::ReadStops(const rm::StopDict &stop_dict) {
  int vertex_id = 0;
  for (auto &[stop, _] : stop_dict) {
    auto &[arrive, depart] = stop_ids_[stop];
    arrive = vertex_id++;
    depart = vertex_id++;
    vertices_[arrive] = Vertex{stop};
    vertices_[depart] = Vertex{stop};
    graph_.AddEdge({
                       .from = arrive,
                       .to = depart,
                       .weight = static_cast<double>(settings_.bus_wait_time)});
    edges_.emplace_back(WaitEdge{});
  }
}

void RouteManager::ReadBuses(const rm::BusDict &bus_dict,
                             const rm::StopDict &stop_dict) {
  for (auto &[bus, bus_info] : bus_dict) {
    auto route = Route(bus_info);
    int stop_count = route.size();
    for (int from = 0; from + 1 < stop_count; ++from) {
      const auto depart = stop_ids_[route[from]].depart;
      double distance = 0.0;
      for (int to = from + 1; to < stop_count; ++to) {
        distance += ComputeRoadDistance({route[to - 1], route[to]}, stop_dict,
                                        false);
        edges_.emplace_back(RoadEdge{
            .bus = bus,
            .span_count = to - from});
        const auto arrive = stop_ids_[route[to]].arrive;
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

  auto vertex_from = it_from->second.arrive;
  auto vertex_to = it_to->second.arrive;
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
          .span_count = ptr_r->span_count});
    } else if (std::holds_alternative<WaitEdge>(edges_[edge_id])) {
      route_info.items.emplace_back(RouteInfo::WaitItem{
          .stop = vertices_[edge.from],
          .time = static_cast<int>(edge.weight)});
    }
  }

  router_->ReleaseRoute(route->id);

  return route_info;
}
}
