#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_UTILS_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_UTILS_H_

#include <vector>
#include <string_view>
#include <map>
#include <unordered_map>

#include "svg/common.h"

#include "sphere.h"

namespace rm::renderer_utils {
struct Route {
  std::vector<std::string_view> route;
  bool is_roundtrip;
};

struct Frame {
  double width, height, padding;
};

using Buses = std::map<std::string_view, Route>;
using Stops = std::map<std::string_view, rm::sphere::Coords>;
using StopCoords = std::unordered_map<std::string_view, svg::Point>;
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_UTILS_H_
