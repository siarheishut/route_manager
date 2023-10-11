#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_H_

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "svg/document.h"

#include "coords_converter.h"
#include "map_renderer_utils.h"
#include "request_types.h"
#include "sphere.h"

namespace rm {
class MapRenderer {
 public:
  static std::unique_ptr<MapRenderer> Create(
      const renderer_utils::Buses &buses,
      const renderer_utils::Stops &stops,
      const RenderingSettings &settings);

  std::string GetMap() const;
 private:
  MapRenderer(const renderer_utils::Buses &buses,
              const renderer_utils::Stops &stops,
              const RenderingSettings &settings);

  void AddBusLinesLayout(
      const renderer_utils::Buses &buses,
      const renderer_utils::Stops &stops,
      const rm::RenderingSettings &settings,
      const renderer_utils::StopCoords &coords);

  void AddBusLabelsLayout(
      const renderer_utils::Buses &buses,
      const renderer_utils::Stops &stops,
      const rm::RenderingSettings &settings,
      const renderer_utils::StopCoords &coords);

  void AddStopPointsLayout(
      const renderer_utils::Stops &stops,
      const rm::RenderingSettings &settings,
      const renderer_utils::StopCoords &coords);

  void AddStopLabelsLayout(
      const renderer_utils::Stops &stops,
      const rm::RenderingSettings &settings,
      const renderer_utils::StopCoords &coords);

  svg::Document map_;
};
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_H_
