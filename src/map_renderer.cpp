#include "map_renderer.h"

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "svg/document.h"
#include "svg/figures.h"

#include "common.h"
#include "coords_converter.h"
#include "map_renderer_utils.h"
#include "request_types.h"
#include "sphere.h"

namespace {
using BaseStops = std::unordered_set<std::string_view>;

// Latitude value range is [-90, 90];
// Longitude value range is [-180, 180].
// We can read about it in the following article:
// https://www.techtarget.com/whatis/definition/latitude-and-longitude.
const double kMinLatitude = -90.0, kMaxLatitude = 90.0,
    kMinLongitude = -180.0, kMaxLongitude = 180.0;

svg::Polyline BusLine(const std::vector<svg::Point> &coords,
                      const svg::Color &color, double line_width) {
  auto bus_line = svg::Polyline{}
      .SetStrokeColor(color)
      .SetStrokeLineCap("round")
      .SetStrokeLineJoin("round")
      .SetStrokeWidth(line_width);
  for (auto point : coords) {
    bus_line.AddPoint(point);
  }
  return bus_line;
}

svg::Section BusName(std::string bus, svg::Point point,
                     const svg::Color &color,
                     const rm::RenderingSettings &settings) {
  svg::SectionBuilder label;
  label.Add(std::move(svg::Text{}
                          .SetPoint(point)
                          .SetOffset(settings.bus_label_offset)
                          .SetFontSize(settings.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(bus)
                          .SetFillColor(settings.underlayer_color)
                          .SetStrokeColor(settings.underlayer_color)
                          .SetStrokeWidth(settings.underlayer_width)
                          .SetStrokeLineCap("round")
                          .SetStrokeLineJoin("round")));
  label.Add(std::move(svg::Text{}
                          .SetPoint(point)
                          .SetOffset(settings.bus_label_offset)
                          .SetFontSize(settings.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(std::move(bus))
                          .SetFillColor(color)));
  return label.Build();
}

svg::Section StopName(std::string stop, svg::Point point,
                      const rm::RenderingSettings &settings) {
  svg::SectionBuilder label;
  label.Add(std::move(svg::Text{}
                          .SetPoint(point)
                          .SetOffset(settings.stop_label_offset)
                          .SetFontSize(settings.stop_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetData(stop)
                          .SetFillColor(settings.underlayer_color)
                          .SetStrokeColor(settings.underlayer_color)
                          .SetStrokeWidth(settings.underlayer_width)
                          .SetStrokeLineCap("round")
                          .SetStrokeLineJoin("round")));
  label.Add(std::move(svg::Text{}
                          .SetPoint(point)
                          .SetOffset(settings.stop_label_offset)
                          .SetFontSize(settings.stop_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetData(std::move(stop))
                          .SetFillColor("black")));
  return label.Build();
}

svg::Circle StopPoint(svg::Point center, double radius) {
  return svg::Circle{}
      .SetFillColor("white")
      .SetCenter(center)
      .SetRadius(radius);
}

std::unordered_map<std::string_view, svg::Point>
CombineCoords(
    const std::vector<std::pair<std::string_view, double>> &x_coords,
    const std::vector<std::pair<std::string_view, double>> &y_coords) {
  std::unordered_map<std::string_view, svg::Point> result;
  for (auto &[stop, x_coord] : x_coords) {
    result[stop].x = x_coord;
  }
  for (auto &[stop, y_coord] : y_coords) {
    result[stop].y = y_coord;
  }
  return result;
}

std::unordered_map<std::string_view, svg::Point> TransformCoords(
    const rm::renderer_utils::Buses &buses,
    rm::renderer_utils::Stops &stops,
    const rm::Frame &frame) {
  using namespace rm::coords_converter;

  BaseStops base_stops;
  rm::Combine(base_stops,
              EndPoints(buses),
              IntersectionsCrossRoute(buses),
              IntersectionsWithinRoute(buses, 3));
  for (auto &bus : buses) {
    stops = Interpolate(std::move(stops), bus.second.route, base_stops);
  }
  auto adjacent_stops = AdjacentStops(buses);

  auto stops_by_lon = SortStops(stops, SortMode::kByLongitude);
  auto layers_by_lon = CompressNonadjacent(stops_by_lon, adjacent_stops);
  auto x_coords = SpreadStops(layers_by_lon,
                              frame.padding,
                              frame.width - frame.padding);

  auto stops_by_lat = SortStops(stops, SortMode::kByLatitude);
  auto layers_by_lat = CompressNonadjacent(stops_by_lat, adjacent_stops);
  auto y_coords = SpreadStops(layers_by_lat,
                              frame.height - frame.padding, frame.padding);

  return CombineCoords(x_coords, y_coords);
}
}

namespace rm {
std::unique_ptr<MapRenderer> MapRenderer::Create(
    const renderer_utils::Buses &buses,
    renderer_utils::Stops stops,
    const RenderingSettings &settings) {
  if (settings.color_palette.empty()) return nullptr;

  for (auto [_, coords] : stops) {
    auto lat = coords.latitude, lon = coords.longitude;
    if (lat < kMinLatitude || lat > kMaxLatitude ||
        lon < kMinLongitude || lon > kMaxLongitude)
      return nullptr;
  }

  for (auto &[bus, route] : buses) {
    if (route.route.size() < 3 || route.route.front() != route.route.back())
      return nullptr;
    for (auto stop : route.route) {
      if (stops.find(stop) == stops.end())
        return nullptr;
    }
    std::unordered_set<std::string_view> route_stops(route.route.begin(),
                                                     route.route.end());
    for (auto endpoint : route.endpoints) {
      if (route_stops.find(endpoint) == route_stops.end())
        return nullptr;
    }
  }

  return std::unique_ptr<MapRenderer>(new MapRenderer(buses, std::move(stops),
                                                      settings));
}

MapRenderer::MapRenderer(
    const renderer_utils::Buses &buses,
    renderer_utils::Stops stops,
    const RenderingSettings &settings)
    : map_(svg::SectionBuilder{}.Build()),
      buses_(ConstructBuses(buses, settings)) {
  auto stop_to_coords = TransformCoords(buses, stops, settings.frame);

  svg::SectionBuilder builder;
  for (auto layer : settings.layers) {
    switch (layer) {
      case MapLayer::kBusLines:
        AddBusLinesLayout(builder,
                          buses_,
                          settings,
                          stop_to_coords);
        break;
      case MapLayer::kBusLabels:
        AddBusLabelsLayout(builder,
                           buses_,
                           settings,
                           stop_to_coords);
        break;
      case MapLayer::kStopPoints:
        AddStopPointsLayout(builder,
                            stops,
                            settings,
                            stop_to_coords);
        break;
      case MapLayer::kStopLabels:
        AddStopLabelsLayout(builder,
                            stops,
                            settings,
                            stop_to_coords);
        break;
    }
  }
  map_ = builder.Build();
}

MapRenderer::Buses MapRenderer::ConstructBuses(
    const renderer_utils::Buses &buses, const RenderingSettings &settings) {
  Buses result;
  int i = 0;
  for (auto &[bus, route] : buses) {
    auto color = settings.color_palette[i];
    i = (i + 1) % settings.color_palette.size();
    auto &bus_info = result[std::string(bus)];
    bus_info = {
        .route = {route.route.begin(), route.route.end()},
        .endpoints = [&r = route]() {
          std::vector<std::string> res;
          auto endpoints = r.endpoints;
          for (auto &stop : r.route) {
            if (auto found = endpoints.find(stop); found != endpoints.end()) {
              res.emplace_back(*found);
              endpoints.erase(found);
            }
          }
          return res;
        }(),
        .color = color
    };
  }
  return result;
}

std::vector<std::string>
MapRenderer::SortBusNames(const MapRenderer::Buses &buses) {
  std::vector<std::string> sorted;
  sorted.reserve(buses.size());
  for (auto &[bus, _] : buses) {
    sorted.push_back(bus);
  }
  std::sort(begin(sorted), end(sorted));
  return sorted;
}

std::vector<svg::Point> MapRenderer::Points(
    const std::vector<std::string> &route,
    const renderer_utils::StopCoords &coords) {
  std::vector<svg::Point> points;
  points.reserve(route.size());
  for (auto &stop : route) {
    points.push_back(coords.at(stop));
  }
  return points;
}

void MapRenderer::AddBusLinesLayout(
    svg::SectionBuilder &builder,
    const MapRenderer::Buses &buses,
    const rm::RenderingSettings &settings,
    const renderer_utils::StopCoords &coords) {
  for (auto &bus : SortBusNames(buses)) {
    auto &bus_info = buses.at(bus);
    builder.Add(BusLine(Points(bus_info.route, coords),
                        bus_info.color, settings.line_width));
  }
}

void MapRenderer::AddBusLabelsLayout(
    svg::SectionBuilder &builder,
    const Buses &buses,
    const rm::RenderingSettings &settings,
    const renderer_utils::StopCoords &coords) {
  for (auto &bus : SortBusNames(buses)) {
    auto &bus_info = buses.at(bus);
    for (auto &endpoint : bus_info.endpoints) {
      builder.Add(BusName(bus, coords.at(endpoint), bus_info.color, settings));
    }
  }
}

void MapRenderer::AddStopPointsLayout(
    svg::SectionBuilder &builder,
    const renderer_utils::Stops &stops,
    const rm::RenderingSettings &settings,
    const renderer_utils::StopCoords &coords) {
  for (auto [stop, _] : stops) {
    builder.Add(StopPoint(coords.at(stop), settings.stop_radius));
  }
}

void MapRenderer::AddStopLabelsLayout(
    svg::SectionBuilder &builder,
    const renderer_utils::Stops &stops,
    const rm::RenderingSettings &settings,
    const renderer_utils::StopCoords &coords) {
  for (auto [stop, _] : stops) {
    builder.Add(StopName(std::string(stop), coords.at(stop), settings));
  }
}

std::string MapRenderer::RenderMap() const {
  svg::Document doc;
  doc.Add(map_);
  std::ostringstream out;
  doc.Render(out);
  return out.str();
}
}
