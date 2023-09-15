#include "request_processor.h"

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
    result.emplace("error_message", std::string("not found"));
    return result;
  }
  result.emplace("total_time", response->time);

  json::List items;
  for (auto &item : response->items) {
    if (std::holds_alternative<RouteResponse::WaitItem>(item)) {
      auto w = std::get<RouteResponse::WaitItem>(item);
      items.emplace_back(json::Dict{
          {"time", w.time},
          {"type", std::string("Wait")},
          {"stop_name", std::move(w.stop)}});
    } else if (std::holds_alternative<RouteResponse::RoadItem>(item)) {
      auto r = std::get<RouteResponse::RoadItem>(item);
      items.emplace_back(json::Dict{
          {"time", r.time},
          {"bus", std::move(r.bus)},
          {"type", std::string("Bus")},
          {"span_count", r.span_count}});
    }
  }
  result.emplace("items", std::move(items));
  return result;
}

json::Dict ToJson(MapResponse response, int id) {
  return json::Dict{{"request_id", id}, {"map", std::move(response)}};
}

json::Dict Process(const BusManager &bm, const GetBusRequest &request) {
  return ToJson(bm.GetBusInfo(request.bus), request.id);
}

json::Dict Process(const BusManager &bm, const GetStopRequest &request) {
  return ToJson(bm.GetStopInfo(request.stop), request.id);
}

json::Dict Process(const BusManager &bm, const GetRouteRequest &request) {
  return ToJson(bm.GetRoute(request.from, request.to), request.id);
}

json::Dict Process(const BusManager &bm, const GetMapRequest &request) {
  return ToJson(bm.GetMap(), request.id);
}

json::List ProcessRequests(const BusManager &bm,
                           std::vector<GetRequest> requests) {
  json::List responses;

  for (auto &request : requests) {
    std::visit([&](auto &&var) {
      responses.emplace_back(Process(bm, var));
    }, request);
  }

  return responses;
}
}
