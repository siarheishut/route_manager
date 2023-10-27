#include "coords_converter.h"

#include <algorithm>
#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "request_types.h"

using namespace std;

namespace rm::coords_converter {
vector <string_view>
SortStops(const renderer_utils::Stops &stops, SortMode mode) {
  vector<pair<double, string_view>> sorted_stops;
  transform(begin(stops), end(stops), back_inserter(sorted_stops),
            [mode](auto &item) {
              if (mode == SortMode::kByLatitude)
                return pair{item.second.latitude, item.first};
              return pair{item.second.longitude, item.first};
            });
  sort(begin(sorted_stops), end(sorted_stops));

  vector<string_view> layers;
  transform(begin(sorted_stops), end(sorted_stops), back_inserter(layers),
            [](auto &item) { return item.second; });
  return layers;
}

std::unordered_set<string_view>
IntersectionsWithinRoute(const renderer_utils::Buses &buses, int count) {
  unordered_set < string_view > base_stops;
  for (auto &[_, route] : buses) {
    unordered_map<string_view, int> stop_freqs;
    for (int i = 0; i < route.route.size(); ++i)
      stop_freqs[route.route[i]] +=
          (!route.is_roundtrip && i < route.route.size() - 1 ? 2 : 1);

    for (auto &[stop, freq] : stop_freqs)
      if (freq >= count)
        base_stops.insert(stop);
  }
  return base_stops;
}

std::unordered_set<string_view>
EndPoints(const renderer_utils::Buses &buses) {
  unordered_set < string_view > base_stops;
  for (auto &[_, route] : buses) {
    base_stops.insert(route.route.front());
    if (!route.is_roundtrip)
      base_stops.insert(route.route.back());
  }
  return base_stops;
}

std::unordered_set<string_view>
IntersectionsCrossRoute(const renderer_utils::Buses &buses) {
  unordered_set < string_view > base_stops, visited;
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

AdjacentList AdjacentStops(const renderer_utils::Buses &buses) {
  AdjacentList adj_stops;
  for (auto &bus : buses) {
    auto &route = bus.second.route;
    for (int i = 1; i < route.size(); ++i) {
      if (route[i] == route[i - 1]) continue;
      adj_stops[route[i]].insert(route[i - 1]);
      adj_stops[route[i - 1]].insert(route[i]);
    }
  }
  return adj_stops;
}

StopLayers CompressNonadjacent(const std::vector<std::string_view> &stops,
                               const AdjacentList &adj_stops) {
  unordered_map<string_view, int> stop_to_layer;
  for (auto &stop : stops) {
    auto &layer = stop_to_layer[stop] = 0;
    auto it = adj_stops.find(stop);
    if (it == adj_stops.end()) continue;
    for (auto &neighbor : it->second) {
      auto found = stop_to_layer.find(neighbor);
      if (found == stop_to_layer.end()) continue;
      layer = max(layer, found->second + 1);
    }
  }

  if (stop_to_layer.empty()) return {};

  auto layer_count = max_element(begin(stop_to_layer), end(stop_to_layer),
                                 [](auto p1, auto p2) {
                                   return p1.second < p2.second;
                                 })->second + 1;

  StopLayers result(layer_count);
  for (auto [stop, idx] : stop_to_layer) {
    result[idx].push_back(stop);
  }
  return result;
}

vector <pair<string_view, double>> SpreadStops(
    const vector <vector<string_view>> &layers,
    double from, double to) {
  int stop_count = layers.size();
  double step = (stop_count == 1) ? 0.0 : (to - from) / (stop_count - 1);
  vector<pair<string_view, double>> result;
  for (int i = 0; i < layers.size(); ++i)
    for (auto stop : layers[i])
      result.emplace_back(stop, from + step * i);
  return result;
}
}
