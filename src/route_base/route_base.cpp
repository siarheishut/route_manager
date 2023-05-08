#include "route_base.h"
#include "src/sphere.h"

#include <utility>
#include <variant>

namespace rm {
double ComputeGeoDistance(const std::vector<rm::Coords> &stops) {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    distance += rm::sphere::CalculateDistance(stops[i - 1], stops[i]);
  }
  return distance;
}

double ComputeRoadDistance(const std::vector<std::string> &stops,
                           const StopDict &dict) {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    auto &dists = dict.at(stops[i - 1]).dists;

    if (auto found = dists.find(stops[i]); found != dists.end())
      distance += found->second;
    else
      distance += sphere::CalculateDistance(dict.at(stops[i - 1]).coords,
                                            dict.at(stops[i]).coords);
  }
  return distance;
}

RouteBase::RouteBase(const rm::StopDict &stop_info,
                     const rm::BusDict &bus_info,
                     const rm::RoutingSettings &routing_settings)
    : settings_(routing_settings), graph_(stop_info.size() * 2) {
  const auto count = stop_info.size() * 2;
  vertices_.resize(count);
  ReadStops(stop_info);
  ReadBuses(bus_info, stop_info);
  router_ = std::make_unique<Router>(std::move(graph_));
}

void RouteBase::ReadStops(const rm::StopDict &stop_dict) {
  int vertex_id = 0;
  for (auto &[stop, _] : stop_dict) {
    auto &[arrive, depart] = stop_to_ids_[stop];
    arrive = vertex_id++;
    depart = vertex_id++;
    vertices_[arrive] = Vertex{stop};
    vertices_[depart] = Vertex{stop};
    graph_.AddEdge({.from = arrive,
                       .to = depart,
                       .weight = static_cast<double>(settings_.bus_wait_time)});
    edges_.emplace_back(WaitEdge{});
  }
}

void RouteBase::ReadBuses(const rm::BusDict &bus_dict,
                          const rm::StopDict &stop_dict) {
  for (auto &[bus, bus_info] : bus_dict) {
    int stop_count = bus_info.stops.size();
    for (int idx_from = 0; idx_from + 1 < stop_count; ++idx_from) {
      const graph::VertexId
          first_id = stop_to_ids_[bus_info.stops[idx_from]].depart;
      double distance = 0.0;
      for (int idx_to = idx_from + 1; idx_to < stop_count; ++idx_to) {
        std::vector<std::string> stops;
        stops.emplace_back(bus_info.stops[idx_to - 1]);
        stops.emplace_back(bus_info.stops[idx_to]);
        distance += ComputeRoadDistance(stops, stop_dict);
        edges_.emplace_back(RoadEdge{
            .bus = bus,
            .span_count = idx_to - idx_from,
        });
        graph_.AddEdge(
            {
                .from = first_id,
                .to = stop_to_ids_[bus_info.stops[idx_to]].arrive,
                .weight = distance / (settings_.bus_velocity * 1000 / 60)
            });
      }
    }
  }
}

std::optional<RouteBase::RouteInfo> RouteBase::FindRoute(const std::string &from,
                                                         const std::string &to) {
  auto vertex_from = stop_to_ids_[from].arrive;
  auto vertex_to = stop_to_ids_[to].arrive;
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
          .stop = vertices_[edge.from].stop,
          .time = static_cast<int>(edge.weight)});
    }
  }
  router_->ReleaseRoute(route->id);
  return route_info;
}

}
