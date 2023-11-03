#include "request_parser.h"

#include <algorithm>
#include <optional>
#include <string_view>
#include <vector>

#include "json.h"
#include "svg/common.h"

#include "color_parser.h"

namespace {
bool IsOffset(const json::Node &node) {
  return node.IsArray() &&
      node.AsArray().size() == 2 &&
      node.AsArray()[0].IsDouble() &&
      node.AsArray()[1].IsDouble();
}

svg::Point AsOffset(json::Node node) {
  return svg::Point{
      .x = node.AsArray()[0].AsDouble(), .y = node.AsArray()[1].AsDouble()};
}

bool IsPalette(const json::Node &node) {
  return node.IsArray() && std::all_of(
      node.AsArray().begin(), node.AsArray().end(), [](auto &item) {
        return rm::IsColor(item);
      });
}

std::vector<svg::Color> AsPalette(json::Node node) {
  std::vector<svg::Color> palette;
  palette.reserve(node.AsArray().size());
  for (auto &item : node.ReleaseArray()) {
    palette.push_back(rm::AsColor(std::move(item)));
  }
  return palette;
}

bool IsLayers(const json::Node &node) {
  return node.IsArray() &&
      std::all_of(node.AsArray().begin(), node.AsArray().end(), [](auto &item) {
        return item.IsString();
      });
}

std::vector<rm::MapLayer> AsLayers(const json::Node &node) {
  std::vector<rm::MapLayer> layers;
  layers.reserve(node.AsArray().size());
  for (auto &item : node.AsArray()) {
    auto &layer = item.AsString();
    if (layer == "bus_lines") {
      layers.push_back(rm::MapLayer::kBusLines);
    } else if (layer == "bus_labels") {
      layers.push_back(rm::MapLayer::kBusLabels);
    } else if (layer == "stop_points") {
      layers.push_back(rm::MapLayer::kStopPoints);
    } else if (layer == "stop_labels") {
      layers.push_back(rm::MapLayer::kStopLabels);
    }
  }
  return layers;
}
}

namespace rm {
std::optional<RoutingSettings> ParseRoutingSettings(json::Dict settings) {
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

std::optional<RenderingSettings> ParseRenderingSettings(json::Dict settings) {
  auto width = settings.find("width");
  auto height = settings.find("height");
  auto padding = settings.find("padding");
  auto stop_radius = settings.find("stop_radius");
  auto line_width = settings.find("line_width");
  auto stop_label_font_size = settings.find("stop_label_font_size");
  auto stop_label_offset = settings.find("stop_label_offset");
  auto underlayer_color = settings.find("underlayer_color");
  auto underlayer_width = settings.find("underlayer_width");
  auto color_palette = settings.find("color_palette");
  auto bus_label_font_size = settings.find("bus_label_font_size");
  auto bus_label_offset = settings.find("bus_label_offset");
  auto layers = settings.find("layers");

  if (width == settings.end() || height == settings.end() ||
      padding == settings.end() || stop_radius == settings.end() ||
      line_width == settings.end() || stop_label_font_size == settings.end() ||
      stop_label_offset == settings.end() ||
      underlayer_color == settings.end() ||
      underlayer_width == settings.end() || color_palette == settings.end() ||
      bus_label_font_size == settings.end() ||
      bus_label_offset == settings.end() ||
      layers == settings.end()) {
    return std::nullopt;
  }

  if (!width->second.IsDouble() || !height->second.IsDouble() ||
      !padding->second.IsDouble() || !stop_radius->second.IsDouble() ||
      !line_width->second.IsDouble() || !stop_label_font_size->second.IsInt() ||
      !IsOffset(stop_label_offset->second) ||
      !IsColor(underlayer_color->second) ||
      !underlayer_width->second.IsDouble() ||
      !IsPalette(color_palette->second) ||
      !bus_label_font_size->second.IsInt() ||
      !IsOffset(bus_label_offset->second) ||
      !IsLayers(layers->second))
    return std::nullopt;

  RenderingSettings rs;
  rs.frame = rm::Frame{
      .width = width->second.AsDouble(),
      .height = height->second.AsDouble(),
      .padding = padding->second.AsDouble(),
  },
      rs.stop_radius = stop_radius->second.AsDouble();
  rs.line_width = line_width->second.AsDouble();
  rs.stop_label_font_size = stop_label_font_size->second.AsInt();
  rs.stop_label_offset = AsOffset(std::move(stop_label_offset->second));
  rs.underlayer_color = AsColor(std::move(underlayer_color->second));
  rs.underlayer_width = underlayer_width->second.AsDouble();
  rs.color_palette = AsPalette(std::move(color_palette->second));
  rs.bus_label_font_size = bus_label_font_size->second.AsInt();
  rs.bus_label_offset = AsOffset(bus_label_offset->second);
  rs.layers = AsLayers(layers->second);

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
  br.is_roundtrip = is_roundtrip->second.AsBool();
  for (auto &stop : stops->second.ReleaseArray()) {
    br.stops.push_back(stop.ReleaseString());
  }

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
  if (!type->second.IsString()) return std::nullopt;

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
