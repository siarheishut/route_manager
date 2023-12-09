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
using AdjacentList = std::unordered_map<std::string_view,
                                        std::unordered_set<std::string_view>>;

using StopLayers = std::vector<std::vector<std::string_view>>;

enum class SortMode { kByLatitude, kByLongitude };

std::vector<std::string_view>
SortStops(const utils::StopCoords &stops, SortMode mode);

// IntersectionsWithinRoute returns stops that are present within the same route
// at least `count` times.
std::unordered_set<std::string_view>
IntersectionsWithinRoute(const renderer_utils::Buses &buses, int count);

std::unordered_set<std::string_view>
EndPoints(const renderer_utils::Buses &buses);

std::unordered_set<std::string_view>
IntersectionsCrossRoute(const renderer_utils::Buses &buses);

// Interpolates coordinates of the stops along the route:
// distributes the non-base stops of the route at equal distances
// from each other along the line between nearest preceding and
// succeeding base stops in the route.
// Doesn't modify base stops' coordinates.
// The base_stops must contain the first and the last route's stop.
// Empty route is always valid and leaves stops unmodified.
utils::StopCoords Interpolate(
    utils::StopCoords stops,
    const std::vector<std::string_view> &route,
    const std::unordered_set<std::string_view> &base_stops);

AdjacentList AdjacentStops(const renderer_utils::Buses &buses);

StopLayers CompressNonadjacent(const std::vector<std::string_view> &stops,
                               const AdjacentList &adj_stops);

std::vector<std::pair<std::string_view, double>> SpreadStops(
    const std::vector<std::vector<std::string_view>> &layers,
    double from, double to);
}

#endif //ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
