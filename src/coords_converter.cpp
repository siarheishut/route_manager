#include "coords_converter.h"

#include <map>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "svg/common.h"

#include "request_types.h"

namespace rm {
std::unordered_map<std::string_view, svg::Point> ConvertCoords(
    const std::map<std::string_view, rm::sphere::Coords> &stops,
    const rm::RenderingSettings &settings) {
  std::unordered_map<std::string_view, svg::Point> result;

  int stop_count = stops.size();
  if (stop_count == 0) return result;
  if (stop_count == 1) {
    result[stops.cbegin()->first] =
        {.x = settings.padding, .y = settings.height - settings.padding};
    return result;
  }

  std::vector<std::pair<double, std::string_view>> stops_by_lat, stops_by_lon;
  stops_by_lat.reserve(stop_count);
  stops_by_lon.reserve(stop_count);
  for (const auto &[stop, coords] : stops) {
    stops_by_lat.emplace_back(coords.latitude, stop);
    stops_by_lon.emplace_back(coords.longitude, stop);
  }
  std::sort(begin(stops_by_lat), end(stops_by_lat));
  std::sort(begin(stops_by_lon), end(stops_by_lon));

  double x_step = (settings.width - 2 * settings.padding) / (stop_count - 1);
  double y_step = (settings.height - 2 * settings.padding) / (stop_count - 1);
  for (int i = 0; i < stops_by_lat.size(); ++i) {
    result[stops_by_lat[i].second].y =
        settings.height - settings.padding - y_step * i;
  }
  for (int i = 0; i < stops_by_lon.size(); ++i) {
    result[stops_by_lon[i].second].x = settings.padding + x_step * i;
  }

  return result;
}
}
