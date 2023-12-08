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
#include "request_types.h"
#include "sphere.h"
#include "transport_catalog.h"

namespace rm {
class MapRenderer {
 public:
  static std::unique_ptr<MapRenderer> Create(
      std::shared_ptr<TransportCatalog> catalog,
      const utils::RenderingSettings &settings);

  std::string RenderMap() const;

  std::optional<std::string> RenderRoute(const utils::RouteInfo &route_info) const;

 private:
  struct BusInfo {
    std::vector<std::string> route;
    std::vector<std::string> endpoints;
    svg::Color color;
  };
  using Buses = std::unordered_map<std::string, BusInfo>;

  using StopPoints = std::unordered_map<std::string, svg::Point>;

  MapRenderer(std::shared_ptr<TransportCatalog> catalog,
              const utils::RenderingSettings &settings);

  static Buses ConstructBuses(const utils::BusDict &buses,
                              const utils::RenderingSettings &settings);

  static std::vector<std::string> SortBusNames(const Buses &buses);

  bool ValidateRoute(const utils::RouteInfo &route_info) const;

  static std::vector<svg::Point> Points(
      const std::vector<std::string> &route,
      const StopPoints &points);

  static void AddBusLinesLayout(
      svg::SectionBuilder &builder,
      const Buses &buses,
      const rm::utils::RenderingSettings &settings,
      const StopPoints &points);

  static void AddBusLabelsLayout(
      svg::SectionBuilder &builder,
      const Buses &buses,
      const rm::utils::RenderingSettings &settings,
      const StopPoints &points);

  static void AddStopPointsLayout(
      svg::SectionBuilder &builder,
      const utils::StopDict &stops,
      const rm::utils::RenderingSettings &settings,
      const StopPoints &points);

  static void AddStopLabelsLayout(
      svg::SectionBuilder &builder,
      const utils::StopDict &stops,
      const rm::utils::RenderingSettings &settings,
      const StopPoints &points);

  svg::Section BusLinesFor(const utils::RouteInfo::RoadItem &item) const;

  svg::Section BusLabelsFor(const utils::RouteInfo::RoadItem &item) const;

  svg::Section StopPointsFor(const utils::RouteInfo::RoadItem &item) const;

  svg::Section StopLabelsFor(const utils::RouteInfo::RoadItem &item,
                             bool first) const;

  std::shared_ptr<TransportCatalog> catalog_;
  svg::Section map_;
  Buses buses_;
  StopPoints stop_points_;
  rm::utils::RenderingSettings settings_;
};
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_H_
