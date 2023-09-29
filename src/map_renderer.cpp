#include "map_renderer.h"

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "svg/figures.h"

#include "coords_converter.h"
#include "request_types.h"
#include "sphere.h"

namespace {
// Latitude value range is [-90, 90];
// Longitude value range is [-180, 180].
// We can read about it in the following article:
// https://www.techtarget.com/whatis/definition/latitude-and-longitude.
const double kMinLatitude = -90.0, kMaxLatitude = 90.0,
    kMinLongitude = -180.0, kMaxLongitude = 180.0;

svg::Text BusNameUnderlayer(std::string name, svg::Point point,
                            const rm::RenderingSettings &settings) {
  return std::move(svg::Text{}
                       .SetPoint(point)
                       .SetOffset(settings.bus_label_offset)
                       .SetFontSize(settings.bus_label_font_size)
                       .SetFontFamily("Verdana")
                       .SetFontWeight("bold")
                       .SetData(std::move(name))
                       .SetFillColor(settings.underlayer_color)
                       .SetStrokeColor(settings.underlayer_color)
                       .SetStrokeWidth(settings.underlayer_width)
                       .SetStrokeLineCap("round")
                       .SetStrokeLineJoin("round"));
}

svg::Text BusNameText(std::string name, svg::Point point,
                      const svg::Color &color,
                      const rm::RenderingSettings &settings) {
  return std::move(svg::Text{}
                       .SetPoint(point)
                       .SetOffset(settings.bus_label_offset)
                       .SetFontSize(settings.bus_label_font_size)
                       .SetFontFamily("Verdana")
                       .SetFontWeight("bold")
                       .SetData(std::move(name))
                       .SetFillColor(color));
}
}

namespace rm {
std::unique_ptr<MapRenderer> MapRenderer::Create(
    const Buses &buses, const Stops &stops, const RenderingSettings &settings) {
  if (settings.color_palette.empty()) return nullptr;

  for (auto [_, coords] : stops) {
    auto lat = coords.latitude, lon = coords.longitude;
    if (lat < kMinLatitude || lat > kMaxLatitude ||
        lon < kMinLongitude || lon > kMaxLongitude)
      return nullptr;
  }

  for (auto &[bus, route] : buses) {
    if (route.route.size() < 2) return nullptr;
    for (auto stop : route.route) {
      if (stops.find(stop) == stops.end())
        return nullptr;
    }
  }

  return std::unique_ptr<MapRenderer>(new MapRenderer(buses, stops, settings));
}

MapRenderer::MapRenderer(
    const Buses &buses, const Stops &stops, const RenderingSettings &settings) {
  double min_lat = kMaxLatitude, max_lat = kMinLatitude;
  double min_lon = kMaxLongitude, max_lon = kMinLongitude;
  for (auto [stop, stop_point] : stops) {
    min_lon = std::min(min_lon, stop_point.longitude);
    max_lon = std::max(max_lon, stop_point.longitude);
    min_lat = std::min(min_lat, stop_point.latitude);
    max_lat = std::max(max_lat, stop_point.latitude);
  }

  auto converter = CoordsConverter(CoordsConverter::Config{
      .min_lon = min_lon, .max_lon = max_lon,
      .min_lat = min_lat, .max_lat = max_lat,
      .width = settings.width, .height = settings.height,
      .padding = settings.padding});

  for (auto layer : settings.layers) {
    switch (layer) {
      case MapLayer::kBusLines:
        AddBusLinesLayout(buses,
                          stops,
                          settings,
                          converter);
        break;
      case MapLayer::kBusLabels:
        AddBusLabelsLayout(buses,
                           stops,
                           settings,
                           converter);
        break;
      case MapLayer::kStopPoints:
        AddStopPointsLayout(stops,
                            settings,
                            converter);
        break;
      case MapLayer::kStopLabels:
        AddStopLabelsLayout(stops,
                            settings,
                            converter);
        break;
    }
  }
}

void MapRenderer::AddBusLinesLayout(
    const Buses &buses, const Stops &stops,
    const rm::RenderingSettings &settings,
    const rm::CoordsConverter &converter) {
  int i = 0;
  for (auto &[_, route] : buses) {
    auto color = settings.color_palette[i];
    i = (i + 1) % settings.color_palette.size();
    svg::Polyline bus_route;
    bus_route.SetStrokeColor(color)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round")
        .SetStrokeWidth(settings.line_width);
    for (auto stop : route.route) {
      bus_route.AddPoint(converter.Convert(stops.at(stop)));
    }
    if (!route.is_roundtrip) {
      for (int j = static_cast<int>(route.route.size()) - 2; j >= 0; --j) {
        bus_route.AddPoint(converter.Convert(stops.at(route.route[j])));
      }
    }
    map_.Add(std::move(bus_route));
  }
}

void MapRenderer::AddBusLabelsLayout(
    const Buses &buses, const Stops &stops,
    const rm::RenderingSettings &settings,
    const rm::CoordsConverter &converter) {
  int i = 0;
  for (auto &[bus, route] : buses) {
    auto color = settings.color_palette[i];
    i = (i + 1) % settings.color_palette.size();
    auto start = route.route.front();
    auto end = route.route.back();
    auto start_point = converter.Convert(stops.at(start));
    auto end_point = converter.Convert(stops.at(end));

    map_.Add(BusNameUnderlayer(std::string(bus), start_point, settings));
    map_.Add(BusNameText(std::string(bus), start_point, color, settings));

    if (route.is_roundtrip || start == end) continue;
    map_.Add(BusNameUnderlayer(std::string(bus), end_point, settings));
    map_.Add(BusNameText(std::string(bus), end_point, color, settings));
  }
}

void MapRenderer::AddStopPointsLayout(
    const Stops &stops,
    const rm::RenderingSettings &settings,
    const rm::CoordsConverter &converter) {
  for (auto [_, coords] : stops) {
    map_.Add(std::move(svg::Circle{}
                           .SetFillColor("white")
                           .SetCenter(converter.Convert(coords))
                           .SetRadius(settings.stop_radius)));
  }
}

void MapRenderer::AddStopLabelsLayout(
    const Stops &stops,
    const rm::RenderingSettings &settings,
    const rm::CoordsConverter &converter) {
  for (auto [stop, coords] : stops) {
    map_.Add(std::move(svg::Text{}
                           .SetPoint(converter.Convert(coords))
                           .SetOffset(settings.stop_label_offset)
                           .SetFontSize(settings.stop_label_font_size)
                           .SetFontFamily("Verdana")
                           .SetData(std::string(stop))
                           .SetFillColor(settings.underlayer_color)
                           .SetStrokeColor(settings.underlayer_color)
                           .SetStrokeWidth(settings.underlayer_width)
                           .SetStrokeLineCap("round")
                           .SetStrokeLineJoin("round")));
    map_.Add(std::move(svg::Text{}
                           .SetPoint(converter.Convert(coords))
                           .SetOffset(settings.stop_label_offset)
                           .SetFontSize(settings.stop_label_font_size)
                           .SetFontFamily("Verdana")
                           .SetData(std::string(stop))
                           .SetFillColor("black")));
  }
}

std::string MapRenderer::GetMap() const {
  std::ostringstream out;
  map_.Render(out);
  return out.str();
}
}
