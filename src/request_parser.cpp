#include "request_parser.h"

#include <optional>
#include <string_view>
#include <vector>

#include "json.h"

namespace rm {
std::optional<RoutingSettings> ParseSettings(json::Dict settings) {
  auto bus_wait_time = settings.find("bus_wait_time");
  auto bus_velocity = settings.find("bus_velocity");

  if (bus_wait_time == settings.end() || bus_velocity == settings.end())
    return std::nullopt;
  if (!bus_wait_time->second.IsInt() || !bus_velocity->second.IsDouble())
    return std::nullopt;

  RoutingSettings rs;
  rs.bus_wait_time = bus_wait_time->second.AsInt();
  rs.bus_velocity = bus_velocity->second.AsDouble();
  return rs;
}

std::optional<std::vector<PostRequest>> ParseInput(json::List base_requests) {
  std::vector<rm::PostRequest> input_requests;
  for (auto &req : base_requests) {
    if (!req.IsMap()) return std::nullopt;
  }

  input_requests.reserve(base_requests.size());
  for (auto &req : base_requests) {
    if (auto request = rm::ParseInputRequest(req.ReleaseMap()); request)
      input_requests.push_back(std::move(*request));
  }

  return input_requests;
}

std::optional<PostRequest> ParseInputRequest(json::Dict dict) {
  auto type = dict.find("type");

  if (type == dict.end()) return std::nullopt;
  if (!type->second.IsString()) return std::nullopt;

  std::string_view request_type = type->second.AsString();
  if (request_type == "Stop") {
    return ParsePostStopRequest(std::move(dict));
  } else if (request_type == "Bus") {
    return ParsePostBusRequest(std::move(dict));
  }

  return std::nullopt;
}

std::optional<PostBusRequest> ParsePostBusRequest(json::Dict dict) {
  auto name = dict.find("name");
  auto stops = dict.find("stops");
  auto is_roundtrip = dict.find("is_roundtrip");

  if (name == dict.end() || stops == dict.end() ||
      is_roundtrip == dict.end()) {
    return std::nullopt;
  }
  if (!name->second.IsString() || !stops->second.IsArray()
      || !is_roundtrip->second.IsBool()) {
    return std::nullopt;
  }
  if (stops->second.AsArray().size() < 2) {
    return std::nullopt;
  }
  for (auto &stop : stops->second.AsArray())
    if (!stop.IsString()) return std::nullopt;
  if (is_roundtrip->second.AsBool() &&
      (stops->second.AsArray().front() != stops->second.AsArray().back()))
    return std::nullopt;

  PostBusRequest br;
  br.bus = name->second.ReleaseString();
  for (auto &stop : stops->second.ReleaseArray()) {
    br.stops.push_back(stop.ReleaseString());
  }
  if (!is_roundtrip->second.AsBool())
    for (int i = static_cast<int>(br.stops.size()) - 2; i >= 0; --i)
      br.stops.push_back(br.stops[i]);

  return br;
}

std::optional<PostStopRequest> ParsePostStopRequest(json::Dict dict) {
  auto name = dict.find("name");
  auto latitude = dict.find("latitude");
  auto longitude = dict.find("longitude");
  auto road_distances = dict.find("road_distances");

  if (name == dict.end() || latitude == dict.end() ||
      longitude == dict.end() || road_distances == dict.end()) {
    return std::nullopt;
  }
  if (!name->second.IsString() || !latitude->second.IsDouble() ||
      !longitude->second.IsDouble() || !road_distances->second.IsMap()) {
    return std::nullopt;
  }
  for (auto &[stop, dist] : road_distances->second.AsMap())
    if (!dist.IsInt()) return std::nullopt;

  PostStopRequest sr;
  sr.stop = name->second.ReleaseString();
  sr.coords.latitude = latitude->second.AsDouble();
  sr.coords.longitude = longitude->second.AsDouble();

  for (auto &[stop, dist] : road_distances->second.ReleaseMap()) {
    sr.stop_distances.emplace(stop, dist.AsInt());
  }

  return sr;
}

std::optional<std::vector<rm::GetRequest>> ParseOutput(json::List stat_requests) {
  std::vector<rm::GetRequest> output_requests;
  for (auto &req : stat_requests) {
    if (!req.IsMap()) return std::nullopt;
  }
  output_requests.reserve(stat_requests.size());
  for (auto &req : stat_requests) {
    if (auto request = rm::ParseOutputRequest(req.ReleaseMap()); request)
      output_requests.push_back(std::move(*request));
  }

  return output_requests;
}

std::optional<GetRequest> ParseOutputRequest(json::Dict dict) {
  auto type = dict.find("type");

  if (type == dict.end()) return std::nullopt;

  std::string_view request_type = type->second.AsString();
  if (request_type == "Stop") {
    return ParseGetStopRequest(std::move(dict));
  } else if (request_type == "Bus") {
    return ParseGetBusRequest(std::move(dict));
  } else if (request_type == "Route") {
    return ParseGetRouteRequest(std::move(dict));
  } else if (request_type == "Map") {
    return ParseGetMapRequest(std::move(dict));
  }

  return std::nullopt;
}

std::optional<GetBusRequest> ParseGetBusRequest(json::Dict dict) {
  auto name = dict.find("name");
  auto id = dict.find("id");
  if (name == dict.end() || id == dict.end())
    return std::nullopt;
  if (!name->second.IsString() || !id->second.IsInt())
    return std::nullopt;

  GetBusRequest sr;
  sr.bus = name->second.ReleaseString();
  sr.id = id->second.AsInt();

  return sr;
}

std::optional<GetStopRequest> ParseGetStopRequest(json::Dict dict) {
  auto name = dict.find("name");
  auto id = dict.find("id");
  if (name == dict.end() || id == dict.end())
    return std::nullopt;
  if (!name->second.IsString() || !id->second.IsInt())
    return std::nullopt;

  GetStopRequest sr;
  sr.stop = name->second.ReleaseString();
  sr.id = id->second.AsInt();

  return sr;
}

std::optional<GetRouteRequest> ParseGetRouteRequest(json::Dict dict) {
  auto from = dict.find("from");
  auto to = dict.find("to");
  auto id = dict.find("id");
  if (from == dict.end() || to == dict.end() || id == dict.end())
    return std::nullopt;
  if (!from->second.IsString() || !to->second.IsString() || !id->second.IsInt())
    return std::nullopt;

  GetRouteRequest gr;
  gr.from = from->second.ReleaseString();
  gr.to = to->second.ReleaseString();
  gr.id = id->second.AsInt();

  return gr;
}

std::optional<GetMapRequest> ParseGetMapRequest(json::Dict dict) {
  auto id = dict.find("id");
  if (id == dict.end() || !id->second.IsInt())
    return std::nullopt;

  return GetMapRequest{.id = id->second.AsInt()};
}
}
