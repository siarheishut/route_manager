#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_H_

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "svg/document.h"

#include "coords_converter.h"
#include "request_types.h"
#include "sphere.h"

namespace rm {
class MapRenderer {
 public:
  struct Route {
    std::vector<std::string_view> route;
    bool is_roundtrip;
  };
  using Buses = std::map<std::string_view, Route>;
  using Stops = std::map<std::string_view, rm::sphere::Coords>;
  using StopCoords = std::unordered_map<std::string_view, svg::Point>;

  static std::unique_ptr<MapRenderer> Create(
      const Buses &buses, const Stops &stops,
      const RenderingSettings &settings);

  std::string GetMap() const;
 private:
  MapRenderer(const Buses &buses, const Stops &stops,
              const RenderingSettings &settings);

  void AddBusLinesLayout(
      const Buses &buses, const Stops &stops,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  void AddBusLabelsLayout(
      const Buses &buses, const Stops &stops,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  void AddStopPointsLayout(
      const Stops &stops,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  void AddStopLabelsLayout(
      const Stops &stops,
      const rm::RenderingSettings &settings,
      const StopCoords &coords);

  svg::Document map_;
};
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_H_
