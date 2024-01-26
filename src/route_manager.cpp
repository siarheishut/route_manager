#include "route_manager.h"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common.h"
#include "distance_computer.h"
#include "request_types.h"
#include "route_manager_utils.h"
#include "transport_catalog.pb.h"

using namespace rm::utils;
using namespace rm::route_manager;
using namespace std;

namespace {
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

bool ValidateEdges(const ::TransportCatalog::TransportDatabase &db,
                   const ::TransportCatalog::Router &router) {
  auto validate_edge = [&](auto &edge) {
    return (edge.bus_id() < db.buses_size() &&
        edge.from_idx() < edge.to_idx() &&
        edge.from_idx() < db.buses().at(edge.bus_id()).route_size() &&
        edge.to_idx() < db.buses().at(edge.bus_id()).route_size() &&
        db.stops().at(edge.from_idx()).stop_name() !=
            db.stops().at(edge.to_idx()).stop_name());
  };

  return all_of(router.edges().begin(), router.edges().end(), validate_edge);
}

vector<graph::Route> Routes(
    const ::TransportCatalog::Router &router,
    const unordered_map<size_t, size_t> &stop_ids,
    const unordered_map<size_t, size_t> &edge_ids) {
  vector<graph::Route> routes;
  for (auto &proto_route : router.routes()) {
    auto &route = routes.emplace_back();
    auto from = proto_route.from();
    auto to = proto_route.to();
    route.from = ToArrive(stop_ids.at(from));
    route.to = ToArrive(stop_ids.at(to));
    route.prev_edge = edge_ids.at(proto_route.last_edge_id());
  }
  return routes;
}

// All bus_id-s in router.edges are expected to be inside db.buses.
// All to_idx-s and from_idx-s in router.edges are expected to be inside
// a route from the related bus in db.buses,
// while to_idx is expected to be greater than from_idx.
struct GraphBuilder {
  const ::TransportCatalog::TransportDatabase &db;
  const ::TransportCatalog::Router &router;
  const unordered_map<string, size_t> &stop_ids;
  int bus_wait_time;

  vector<rm::route_manager::Edge> BuildEdges() const {
    vector<rm::route_manager::Edge> edges(router.edges_size());
    for (int i = 0; i < stop_ids.size(); ++i) {
      edges.emplace_back(WaitEdge{});
    }
    for (auto &proto_edge : router.edges()) {
      edges.emplace_back(RoadEdge{
          .bus = db.buses().at(proto_edge.bus_id()).bus_name(),
          .start_idx = static_cast<int>(proto_edge.from_idx()),
          .span_count = static_cast<int>(
              proto_edge.to_idx() - proto_edge.from_idx())});
    }
    return edges;
  }

  rm::route_manager::Graph BuildGraph() const {
    rm::route_manager::Graph graph(stop_ids.size() * 2);
    for (auto &[stop, id] : stop_ids) {
      graph.AddEdge({
                        .from = ToArrive(id),
                        .to = ToDepart(id),
                        .weight = static_cast<double>(bus_wait_time)});
    }
    for (auto &proto_edge : router.edges()) {
      auto &bus = db.buses().at(proto_edge.bus_id());
      auto from = bus.route().at(proto_edge.from_idx());
      auto to = bus.route().at(proto_edge.to_idx());
      graph.AddEdge({
                        .from = ToDepart(from),
                        .to = ToArrive(to),
                        .weight = proto_edge.weight()});
    }
    return graph;
  }
};

::TransportCatalog::RoutingSettings ToProto(RoutingSettings s) {
  ::TransportCatalog::RoutingSettings proto_routing_settings;
  proto_routing_settings.set_bus_wait_time(s.bus_wait_time);
  proto_routing_settings.set_bus_velocity(s.bus_velocity);
  return proto_routing_settings;
}

template<typename Container>
unordered_map<string_view, size_t> BusIndices(const Container &cont) {
  int idx = 0;
  unordered_map<string_view, size_t> bus_indices;
  for (auto &bus : cont) {
    bus_indices[bus.bus_name()] = idx++;
  }
  return bus_indices;
}

vector<::TransportCatalog::Edge> ProtoEdges(
    const Graph &graph,
    const vector<Edge> &edges,
    const unordered_map<string_view, size_t> &bus_indices) {
  vector<::TransportCatalog::Edge> result;
  for (int i = 0; i < edges.size(); ++i) {
    if (holds_alternative<WaitEdge>(edges[i]))
      continue;
    auto &edge = get<RoadEdge>(edges[i]);
    auto &graph_edge = graph.GetEdge(i);

    ::TransportCatalog::Edge proto_edge;
    proto_edge.set_weight(graph_edge.weight);
    proto_edge.set_from_idx(edge.start_idx);
    proto_edge.set_to_idx(edge.start_idx + edge.span_count);
    proto_edge.set_bus_id(bus_indices.at(edge.bus));
    result.push_back(std::move(proto_edge));
  }
  return result;
}

// Works due to the fact, that all wait edges are always being maintained before
// all road edges.
unordered_map<size_t, size_t> EdgeIdToProtoEdgeId(
    const vector<rm::route_manager::Edge> &edges, int stop_count) {
  unordered_map<size_t, size_t> result;
  int edge_id = stop_count, proto_edge_id = 0;
  while (edge_id < edges.size()) {
    result[edge_id] = proto_edge_id;
    ++edge_id, ++proto_edge_id;
  }
  return result;
}

// Based on the knowledge that the first half of edges are wait-edges, and the
// second half are route-edges.
unordered_map<size_t, size_t> ProtoEdgeIdToEdgeId(
    const ::TransportCatalog::Router &proto_router, int stop_count) {
  unordered_map<size_t, size_t> result;
  int edge_id = stop_count, proto_edge_id = 0;
  while (proto_edge_id < proto_router.edges_size()) {
    result[proto_edge_id] = edge_id;
    ++edge_id, ++proto_edge_id;
  }
  return result;
}

std::unordered_map<size_t, size_t> ProtoStopIdToStopId(
    const unordered_map<string, size_t> &stop_ids,
    const ::TransportCatalog::TransportDatabase &db) {
  std::unordered_map<size_t, size_t> result;
  for (int i = 0; i < db.stops().size(); ++i) {
    auto &name = db.stops().at(i).stop_name();
    if (stop_ids.find(name) != stop_ids.end()) {
      result[i] = stop_ids.at(name);
    } else {
      std::runtime_error("ProtoStopIdToStopId : stop should have been found");
    }
  }
  return result;
}

vector<::TransportCatalog::Route> ProtoRoutes(
    const Router &router, unordered_map<size_t, size_t> proto_edge_ids) {
  vector<::TransportCatalog::Route> result;
  for (auto [from, to, prev_edge] : router.Serialize()) {
    if (IsDepart(from) || IsDepart(to)) continue;

    ::TransportCatalog::Route proto_route;
    proto_route.set_from(ToStopId(from));
    proto_route.set_to(ToStopId(to));
    proto_route.set_last_edge_id(proto_edge_ids[prev_edge]);
    result.push_back(std::move(proto_route));
  }
  return result;
}
}

namespace rm {
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

::TransportCatalog::Router RouteManager::Serialize(
    const ::TransportCatalog::TransportDatabase &proto_db) const {
  ::TransportCatalog::Router proto_router;
  *proto_router.mutable_settings() = ToProto(settings_);

  for (auto &proto_edge : ProtoEdges(graph_,
                                     edges_,
                                     BusIndices(proto_db.buses()))) {
    *proto_router.add_edges() = std::move(proto_edge);
  }

  auto proto_edge_ids = EdgeIdToProtoEdgeId(edges_, catalog_->Stops().size());
  for (auto &route : ProtoRoutes(*router_, std::move(proto_edge_ids))) {
    *proto_router.add_routes() = std::move(route);
  }

  return proto_router;
}

unique_ptr<RouteManager> RouteManager::Deserialize(
    const ::TransportCatalog::TransportCatalog &proto_catalog,
    shared_ptr<TransportCatalog> catalog) {
  auto &proto_db = proto_catalog.database();
  auto &proto_router = proto_catalog.router();
  auto &stops = catalog->Stops();

  if (!proto_catalog.has_router() || !proto_router.has_settings() ||
      !ValidateEdges(proto_db, proto_router))
    return nullptr;

  Graph graph{0};
  utils::RoutingSettings settings{
      static_cast<int>(proto_router.settings().bus_wait_time()),
      proto_router.settings().bus_velocity()};
  vector<Edge> edges;
  vector<string> stop_names(stops.size());
  unordered_map<string, size_t> stop_ids;

  int stop_id = 0;
  for (auto &[stop, _] : stops) {
    stop_ids[stop] = stop_id;
    stop_names[stop_id] = stop;
    ++stop_id;
  }
  GraphBuilder
      graph_builder{proto_db, proto_router, stop_ids, settings.bus_wait_time};
  graph = graph_builder.BuildGraph();
  edges = graph_builder.BuildEdges();

  auto routes = Routes(proto_router,
                       ProtoStopIdToStopId(stop_ids, proto_db),
                       ProtoEdgeIdToEdgeId(proto_router,
                                           proto_db.stops_size()));
  auto router = Router::Deserialize(graph, routes);
  if (!router) return nullptr;

  return unique_ptr<RouteManager>(new RouteManager(Params{
      std::move(catalog), std::move(router), std::move(graph),
      settings, std::move(edges), std::move(stop_names), std::move(stop_ids)}));
}

RouteManager::RouteManager(Params config)
    : catalog_(std::move(config.catalog)),
      graph_(std::move(config.graph)),
      router_(std::move(config.router)),
      settings_(config.settings),
      edges_(std::move(config.edges)),
      stop_names_(std::move(config.stop_names)),
      stop_ids_(std::move(config.stop_ids)) {}

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

optional<RouteInfo> RouteManager::FindRoute(const string &from,
                                            const string &to) {
  auto it_from = stop_ids_.find(from);
  auto it_to = stop_ids_.find(to);
  if (it_from == stop_ids_.end() || it_to == stop_ids_.end())
    return nullopt;

  auto vertex_from = ToArrive(it_from->second);
  auto vertex_to = ToArrive(it_to->second);
  auto route = router_->BuildRoute(vertex_from, vertex_to);
  if (!route) return nullopt;

  RouteInfo route_info;
  auto route_id = route->id;
  route_info.time = route->weight;
  for (int i = 0; i < route->edge_count; ++i) {
    auto edge_id = router_->GetRouteEdge(route_id, i);
    auto &edge = graph_.GetEdge(edge_id);

    if (auto ptr_r = get_if<RoadEdge>(&edges_[edge_id])) {
      route_info.items.emplace_back(RouteInfo::RoadItem{
          .bus = ptr_r->bus,
          .time = edge.weight,
          .start_idx = ptr_r->start_idx,
          .span_count = ptr_r->span_count});
    } else if (holds_alternative<WaitEdge>(edges_[edge_id])) {
      route_info.items.emplace_back(RouteInfo::WaitItem{
          .stop = stop_names_[ToStopId(edge.from)],
          .time = static_cast<int>(edge.weight)});
    }
  }

  router_->ReleaseRoute(route->id);

  return route_info;
}
}
