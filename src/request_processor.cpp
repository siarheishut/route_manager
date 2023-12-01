#include "request_processor.h"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "bus_manager.h"
#include "map_renderer.h"
#include "map_renderer_utils.h"
#include "response_types.h"

using namespace rm::utils;

namespace {
auto MapRendererParams(const std::vector<PostRequest> &requests) {
  using namespace std;
  using namespace rm;

  map<string_view, Route> buses;
  map<string_view, sphere::Coords> stops;
  for (auto &request : requests) {
    if (auto bus = get_if<PostBusRequest>(&request)) {
      buses.emplace(
          bus->bus,
          Route{
              .route = {begin(bus->stops), end(bus->stops)},
              .endpoints = {begin(bus->endpoints), end(bus->endpoints)}});
    } else if (auto stop = get_if<PostStopRequest>(&request)) {
      stops[stop->stop] = stop->coords;
    }
  }
  return pair{move(buses), move(stops)};
}
}

namespace rm {
json::Dict ToJson(std::optional<BusResponse> response, int id) {
  json::Dict result;

  result.emplace("request_id", id);
  if (!response) {
    result.emplace("error_message", std::string("not found"));
    return result;
  }
  result.emplace("route_length", response->length);
  result.emplace("curvature", response->curvature);
  result.emplace("stop_count", response->stop_count);
  result.emplace("unique_stop_count", response->unique_stop_count);
  return result;
}

json::Dict ToJson(std::optional<StopResponse> response, int id) {
  json::Dict result;

  result.emplace("request_id", id);
  if (!response) {
    result.emplace("error_message", std::string("not found"));
  } else {
    result.emplace("buses",
                   json::List(std::move_iterator(response->buses.begin()),
                              std::move_iterator(response->buses.end())));
  }

  return result;
}

json::Dict ToJson(std::optional<RouteResponse> response,
                  std::optional<std::string> map, int id) {
  json::Dict result;

  result.emplace("request_id", id);
  if (!response) {
    result.emplace("error_message", "not found");
    return result;
  }
  result.emplace("total_time", response->time);

  json::List items;
  for (auto &item : response->items) {
    if (std::holds_alternative<RouteResponse::WaitItem>(item)) {
      auto w = std::get<RouteResponse::WaitItem>(item);
      items.emplace_back(json::Dict{
          {"time", w.time},
          {"type", "Wait"},
          {"stop_name", std::move(w.stop)}});
    } else if (std::holds_alternative<RouteResponse::RoadItem>(item)) {
      auto r = std::get<RouteResponse::RoadItem>(item);
      items.emplace_back(json::Dict{
          {"time", r.time},
          {"bus", std::move(r.bus)},
          {"type", "Bus"},
          {"span_count", r.span_count}});
    };
  }
  result.emplace("items", std::move(items));
  if (!map.has_value()) {
    result.emplace("error_message", "invalid route info");
  } else {
    result.emplace("map", *map);
  }
  return result;
}

json::Dict ToJson(MapResponse resp, int id) {
  return json::Dict{{"request_id", id}, {"map", resp.map}};
}

std::unique_ptr<Processor> Processor::Create(
    std::vector<PostRequest> requests,
    const RoutingSettings &routing_settings,
    const RenderingSettings &rendering_settings) {
  auto bus_manager = BusManager::Create(requests, routing_settings);
  if (!bus_manager) return nullptr;

  auto [buses, stops] = MapRendererParams(requests);
  auto map_renderer = MapRenderer::Create(buses, std::move(stops),
                                          rendering_settings);
  if (!map_renderer) return nullptr;

  return std::unique_ptr<Processor>(new Processor(std::move(bus_manager),
                                                  std::move(map_renderer)));
}

json::List Processor::Process(const std::vector<GetRequest> &requests) const {
  json::List responses;

  for (auto &request : requests) {
    std::visit([&](auto &&var) {
      responses.emplace_back(Process(var));
    }, request);
  }

  return responses;
}

Processor::Processor(std::unique_ptr<BusManager> bus_manager,
                     std::unique_ptr<MapRenderer> map_renderer)
    : bus_manager_(std::move(bus_manager)),
      map_renderer_(std::move(map_renderer)) {}

json::Dict Processor::Process(const GetBusRequest &request) const {
  return ToJson(bus_manager_->GetBusInfo(request.bus), request.id);
}

json::Dict Processor::Process(const GetStopRequest &request) const {
  return ToJson(bus_manager_->GetStopInfo(request.stop), request.id);
}

json::Dict Processor::Process(const GetRouteRequest &request) const {
  auto route_info = bus_manager_->GetRoute(request.from, request.to);
  if (!route_info.has_value())
    return ToJson(std::nullopt, "", request.id);
  auto map = map_renderer_->RenderRoute(*route_info);
  return ToJson(*route_info, map, request.id);
}

json::Dict Processor::Process(const GetMapRequest &request) const {
  return ToJson(MapResponse{.map = map_renderer_->RenderMap()}, request.id);
}
}
