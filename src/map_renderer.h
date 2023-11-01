#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "svg/figures.h"
#include "svg/common.h"

#include "common.h"
#include "coords_converter.h"
#include "map_renderer_utils.h"
#include "request_types.h"
#include "sphere.h"

namespace rm {
class MapRenderer {
 public:
  static std::unique_ptr<MapRenderer> Create(
      const renderer_utils::Buses &buses,
      renderer_utils::Stops stops,
      const RenderingSettings &settings);

  std::string RenderMap() const;

  std::optional<std::string> RenderRoute(const RouteInfo &route_info) const;

 private:
  struct BusInfo {
    std::vector<std::string> route;
    std::vector<std::string> endpoints;
    svg::Color color;
  };
  using Buses = std::unordered_map<std::string, BusInfo>;

  using StopCoords = std::unordered_map<std::string, svg::Point>;

  MapRenderer(const renderer_utils::Buses &buses,
              renderer_utils::Stops stops,
              const RenderingSettings &settings);

  static Buses ConstructBuses(const renderer_utils::Buses &buses,
                              const RenderingSettings &settings);

  static std::vector<std::string> SortBusNames(const Buses &buses);

  bool ValidateRoute(const RouteInfo &route_info) const;

  static std::vector<svg::Point> Points(
      const std::vector<std::string> &route,
      const StopCoords &coords);

  static void AddBusLinesLayout(
      svg::SectionBuilder &builder,
      const Buses &buses,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  static void AddBusLabelsLayout(
      svg::SectionBuilder &builder,
      const Buses &buses,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  static void AddStopPointsLayout(
      svg::SectionBuilder &builder,
      const renderer_utils::Stops &stops,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  static void AddStopLabelsLayout(
      svg::SectionBuilder &builder,
      const renderer_utils::Stops &stops,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  svg::Section BusLinesFor(const RouteInfo::RoadItem &item) const;

  svg::Section BusLabelsFor(const RouteInfo::RoadItem &item) const;

  svg::Section StopPointsFor(const RouteInfo::RoadItem &item) const;

  svg::Section StopLabelsFor(const RouteInfo::RoadItem &item, bool first) const;

  svg::Section map_;
  Buses buses_;
  StopCoords stop_coords_;
  rm::RenderingSettings settings_;
};
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_H_
