#include "map_renderer.h"

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
}

namespace rm {
std::unique_ptr<MapRenderer> MapRenderer::Create(
    std::map<std::string_view, std::vector<std::string_view>> buses,
    std::map<std::string_view, sphere::Coords> stops,
    const RenderingSettings &settings) {
  if (settings.color_palette.empty()) return nullptr;

  for (auto [_, coords] : stops) {
    auto lat = coords.latitude, lon = coords.longitude;
    if (lat < kMinLatitude || lat > kMaxLatitude ||
        lon < kMinLongitude || lon > kMaxLongitude)
      return nullptr;
  }

  for (auto &[bus, route] : buses) {
    if (route.empty()) return nullptr;
    for (auto stop : route) {
      if (stops.find(stop) == stops.end())
        return nullptr;
    }
  }

  return std::unique_ptr<MapRenderer>(
      new MapRenderer(std::move(buses), std::move(stops), settings));
}

MapRenderer::MapRenderer(
    std::map<std::string_view, std::vector<std::string_view>> buses,
    std::map<std::string_view, sphere::Coords> stops,
    const RenderingSettings &settings) {
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

  int i = 0;
  for (auto &[_, route] : buses) {
    auto color = settings.color_palette[i];
    i = (i + 1) % settings.color_palette.size();
    svg::Polyline bus_route;
    bus_route.SetStrokeColor(color)
        .SetStrokeLineCap("round")
        .SetStrokeLineJoin("round")
        .SetStrokeWidth(settings.line_width);
    for (auto stop : route) {
      bus_route.AddPoint(converter.Convert(stops[stop]));
    }
    map_.Add(std::move(bus_route));
  }

  for (auto [_, coords] : stops) {
    map_.Add(std::move(svg::Circle{}
                           .SetFillColor("white")
                           .SetCenter(converter.Convert(coords))
                           .SetRadius(settings.stop_radius)));
  }

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
