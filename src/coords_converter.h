#ifndef ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
#define ROOT_MANAGER_SRC_COORDS_CONVERTER_H_

#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "svg/common.h"

#include "map_renderer_utils.h"
#include "request_types.h"
#include "sphere.h"

namespace rm::coords_converter {
enum class SortMode { kByLatitude, kByLongitude };

struct Frame { double width, height, padding; };

std::vector<std::string_view>
SortStops(const renderer_utils::Stops &stops, SortMode mode);

std::unordered_set<std::string_view>
BaseStops(const rm::renderer_utils::Buses &buses);

renderer_utils::Stops
Interpolate(const renderer_utils::Buses &buses,
            const std::unordered_set<std::string_view> &base_stops,
            rm::renderer_utils::Stops stops);

std::vector<std::pair<std::string_view, std::string_view>>
AdjacentStops(const renderer_utils::Buses &buses);

std::vector<std::vector<std::string_view>> CompressNonadjacent(
    const std::vector<std::string_view> &stops,
    const std::vector<std::pair<std::string_view,
                                std::string_view>> &neighbors);

std::vector<std::pair<std::string_view, double>> SpreadStops(
    const std::vector<std::vector<std::string_view>> &layers,
    double from, double to);
}

#endif //ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
