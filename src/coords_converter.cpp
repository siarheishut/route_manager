#include "coords_converter.h"

#include <algorithm>
#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "request_types.h"

namespace rm::coords_converter {
std::vector<std::string_view>
SortStops(const renderer_utils::Stops &stops, SortMode mode) {
  std::vector<std::pair<double, std::string_view>> sorted_stops;
  transform(begin(stops), end(stops), back_inserter(sorted_stops),
            [mode](auto &item) {
              if (mode == SortMode::kByLatitude)
                return std::pair{item.second.latitude, item.first};
              return std::pair{item.second.longitude, item.first};
            });
  sort(begin(sorted_stops), end(sorted_stops));

  std::vector<std::string_view> layers;
  transform(begin(sorted_stops), end(sorted_stops), back_inserter(layers),
            [](auto &item) { return item.second; });
  return layers;
}

std::unordered_set<std::string_view>
IntersectionsWithinRoute(const renderer_utils::Buses &buses, int count) {
  std::unordered_set < std::string_view > base_stops;
  for (auto &[_, route] : buses) {
    std::unordered_map<std::string_view, int> stop_freqs;
    for (int i = 0; i < route.route.size(); ++i)
      stop_freqs[route.route[i]] +=
          (!route.is_roundtrip && i < route.route.size() - 1 ? 2 : 1);

    for (auto &[stop, freq] : stop_freqs)
      if (freq >= count)
        base_stops.insert(stop);
  }
  return base_stops;
}

std::unordered_set<std::string_view>
EndPoints(const renderer_utils::Buses &buses) {
  std::unordered_set < std::string_view > base_stops;
  for (auto &[_, route] : buses) {
    base_stops.insert(route.route.front());
    if (!route.is_roundtrip)
      base_stops.insert(route.route.back());
  }
  return base_stops;
}

std::unordered_set<std::string_view>
IntersectionsCrossRoute(const renderer_utils::Buses &buses) {
  std::unordered_set < std::string_view > base_stops, visited;
  for (auto &[_, route] : buses) {
    for (auto stop : route.route)
      if (visited.find(stop) != visited.end())
        base_stops.insert(stop);

    for (auto stop : route.route) {
      visited.insert(stop);
    }
  }
  return base_stops;
}

renderer_utils::Stops Interpolate(
    renderer_utils::Stops stops,
    const std::vector<std::string_view> &route,
    const std::unordered_set<std::string_view> &base_stops) {
  auto is_base =
      [&](auto s) { return base_stops.find(s) != end(base_stops); };
  auto find_next = [&](int i) {
    auto it = find_if(begin(route) + i, end(route), is_base);
    return it - begin(route);
  };

  int left = 0, right = 0;
  for (int i = 0; i < route.size(); ++i) {
    if (i == right) {
      left = right;
      right = find_next(right + 1);
      continue;
    }

    auto stop = route[i];
    auto lon_step = (stops[route[right]].longitude -
        stops[route[left]].longitude) / (right - left);
    auto lat_step = (stops[route[right]].latitude -
        stops[route[left]].latitude) / (right - left);

    stops[stop].longitude = stops[route[left]].longitude +
        lon_step * (i - left);
    stops[stop].latitude = stops[route[left]].latitude +
        lat_step * (i - left);
  }
  return std::move(stops);
}

std::vector<std::pair<std::string_view, std::string_view>>
AdjacentStops(const renderer_utils::Buses &buses) {
  std::vector<std::pair<std::string_view, std::string_view>> result;
  for (auto &[_, route] : buses) {
    for (int i = 1; i < route.route.size(); ++i) {
      auto prev = route.route[i - 1];
      auto curr = route.route[i];
      if (prev == curr) continue;
      result.emplace_back(prev, curr);
    }
  }
  return result;
}

std::vector<std::vector<std::string_view>> CompressNonadjacent(
    const std::vector<std::string_view> &stops,
    const std::vector<std::pair<std::string_view,
                                std::string_view>> &neighbors) {
  std::unordered_map<std::string_view, int> stop_to_layer;
  for (int i = 0; i < stops.size(); ++i)
    stop_to_layer[stops[i]] = i;

  std::vector<int> max_next(stops.size(), static_cast<int>(stops.size()) - 1);

  for (auto &[prev, curr] : neighbors) {
    auto prev_layer = stop_to_layer[prev];
    auto curr_layer = stop_to_layer[curr];
    if (prev_layer > curr_layer) std::swap(prev_layer, curr_layer);
    max_next[prev_layer] = std::min(max_next[prev_layer], curr_layer - 1);
  }

  std::vector<std::vector<std::string_view>> res;
  int last = -1;
  for (int layer = 0; layer < stops.size(); ++layer) {
    if (layer > last) {
      res.push_back({stops[layer]});
      last = max_next[layer];
      continue;
    }
    last = std::min(last, max_next[layer]);
    res.back().push_back(stops[layer]);
  }
  return res;
}

std::vector<std::pair<std::string_view, double>> SpreadStops(
    const std::vector<std::vector<std::string_view>> &layers,
    double from, double to) {
  int stop_count = layers.size();
  double step = (stop_count == 1) ? 0.0 : (to - from) / (stop_count - 1);
  std::vector<std::pair<std::string_view, double>> result;
  for (int i = 0; i < layers.size(); ++i)
    for (auto stop : layers[i])
      result.emplace_back(stop, from + step * i);
  return result;
}
}
