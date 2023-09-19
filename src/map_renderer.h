#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_H_

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "svg/document.h"

#include "request_types.h"
#include "sphere.h"

namespace rm {
class MapRenderer {
 public:
  static std::unique_ptr<MapRenderer> Create(
      std::map<std::string_view, std::vector<std::string_view>> buses,
      std::map<std::string_view, sphere::Coords> stops,
      const RenderingSettings &settings);

  std::string GetMap() const;
 private:
  MapRenderer(std::map<std::string_view, std::vector<std::string_view>> buses,
              std::map<std::string_view, sphere::Coords> stops,
              const RenderingSettings &settings);

  svg::Document map_;
};
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_H_
