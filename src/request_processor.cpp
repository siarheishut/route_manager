#include "request_processor.h"
#include "bus_manager.h"

#include <variant>

namespace rm {
json::List ToJson(std::vector<RouteInfo::Item> response_items) {
  json::List items;
  for (auto &item : response_items) {
    if (std::holds_alternative<RouteInfo::WaitItem>(item)) {
      auto ptr_w = std::get<RouteInfo::WaitItem>(item);
      items.emplace_back(json::Dict{
          {"time", ptr_w.time},
          {"type", "Wait"},
          {"stop_name", std::move(ptr_w.stop)}});
    } else if (std::holds_alternative<RouteInfo::RoadItem>(item)) {
      auto ptr_r = std::get<RouteInfo::RoadItem>(item);
      items.emplace_back(json::Dict{
          {"time", ptr_r.time},
          {"bus", std::move(ptr_r.bus)},
          {"type", "Bus"},
          {"span_count", ptr_r.span_count}});
    };
  }
  return items;
}

json::Dict ToJson(std::optional<RouteInfo> response, int id) {
  json::Dict result;

  result.emplace("request_id", id);
  if (!response) {
    result.emplace("error_message", "not found");
    return result;
  }
  result.emplace("total_time", response->time);
  result.emplace("items", ToJson(std::move(response->items)));
  return result;
}

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

json::Dict Process(const BusManager &bm, const GetBusRequest &request) {
  return ToJson(bm.GetBusInfo(request.bus), request.id);
}

json::Dict Process(const BusManager &bm, const GetStopRequest &request) {
  return ToJson(bm.GetStopInfo(request.stop), request.id);
}

json::Dict Process(const BusManager &bm, const GetRouteRequest &request) {
  return ToJson(bm.FindRoute(request.from, request.to), request.id);
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
