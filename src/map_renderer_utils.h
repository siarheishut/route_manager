#ifndef ROOT_MANAGER_SRC_MAP_RENDERER_UTILS_H_
#define ROOT_MANAGER_SRC_MAP_RENDERER_UTILS_H_

#include <vector>
#include <string_view>
#include <map>
#include <unordered_map>

#include "svg/common.h"

#include "common.h"
#include "sphere.h"

namespace rm::renderer_utils {
using Buses = std::map<std::string_view, rm::Route>;
using Stops = std::map<std::string_view, rm::sphere::Coords>;
}

#endif // ROOT_MANAGER_SRC_MAP_RENDERER_UTILS_H_
