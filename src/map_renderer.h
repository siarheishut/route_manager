#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_H_

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "request_types.h"
#include "sphere.h"

namespace rm {
class MapRenderer {
 public:
  static std::unique_ptr<MapRenderer> Create(
      std::map<std::string_view, std::vector<std::string_view>> buses,
      std::map<std::string_view, sphere::Coords> stops,
      const RenderingSettings &settings);
 private:
  MapRenderer(std::map<std::string_view, std::vector<std::string_view>> buses,
              std::map<std::string_view, sphere::Coords> stops,
              const RenderingSettings &settings);
};
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_H_
