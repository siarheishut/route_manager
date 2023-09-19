#include "map_renderer.h"

#include <map>
#include <memory>
#include <string_view>
#include <vector>

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
    const RenderingSettings &settings) {}
}
