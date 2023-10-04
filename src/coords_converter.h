#ifndef ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
#define ROOT_MANAGER_SRC_COORDS_CONVERTER_H_

#include <map>
#include <string_view>
#include <unordered_map>

#include "svg/common.h"

#include "request_types.h"
#include "sphere.h"

namespace rm {
std::unordered_map<std::string_view, svg::Point> ConvertCoords(
    const std::map<std::string_view, rm::sphere::Coords> &stops,
    const rm::RenderingSettings &settings);
}

#endif //ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
