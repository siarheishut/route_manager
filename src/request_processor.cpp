#include "request_processor.h"

#include <memory>
#include <string>
#include <variant>

#include "bus_manager.h"

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

json::Dict ToJson(std::optional<RouteResponse> response, int id) {
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
  return result;
}

std::unique_ptr<Processor> Processor::Create(
    std::vector<PostRequest> requests,
    const rm::RoutingSettings &routing_settings) {
  auto bus_manager = BusManager::Create(requests, routing_settings);
  if (!bus_manager) return nullptr;

  return std::unique_ptr<Processor>(new Processor(std::move(bus_manager)));
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

Processor::Processor(std::unique_ptr<BusManager> bus_manager)
    : bus_manager_(std::move(bus_manager)) {}

json::Dict Processor::Process(const GetBusRequest &request) const {
  return ToJson(bus_manager_->GetBusInfo(request.bus), request.id);
}

json::Dict Processor::Process(const GetStopRequest &request) const {
  return ToJson(bus_manager_->GetStopInfo(request.stop), request.id);
}

json::Dict Processor::Process(const GetRouteRequest &request) const {
  return ToJson(bus_manager_->GetRoute(request.from, request.to), request.id);
}

json::Dict Processor::Process(const GetMapRequest &request) const {
  // TODO(siarheishut): implement.
  return json::Dict{};
}
}
