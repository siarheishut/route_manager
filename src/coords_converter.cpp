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

unordered_set <string_view> BaseStops(const renderer_utils::Buses &buses) {
  unordered_set < string_view > base_stops, visited;

  for (auto &[_, route] : buses) {
    base_stops.insert(route.route.front());
    if (!route.is_roundtrip)
      base_stops.insert(route.route.back());
  }

  for (auto &[_, route] : buses) {
    for (auto stop : route.route)
      if (visited.find(stop) != visited.end())
        base_stops.insert(stop);

    for (auto stop : route.route) {
      visited.insert(stop);
    }
  }

  for (auto &[_, route] : buses) {
    unordered_map<string_view, int> stop_freqs;

    for (auto stop : route.route) {
      if (++stop_freqs[stop] == 3)
        base_stops.insert(stop);
    }

    if (!route.is_roundtrip) {
      for (int i = 1; i < route.route.size() - 1; ++i) {
        if (++stop_freqs[route.route[i]] == 3) {
          base_stops.insert(route.route[i]);
        }
      }
    }

  }
  return base_stops;
}

renderer_utils::Stops Interpolate(const renderer_utils::Buses &buses,
                                  const unordered_set <string_view> &base_stops,
                                  rm::renderer_utils::Stops stops) {
  for (auto &item : buses) {
    auto route = item.second.route;
    if (!item.second.is_roundtrip)
      for (int i = static_cast<int>(route.size()) - 2; i >= 0; --i) {
        route.push_back(route[i]);
      }

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
  }
  return std::move(stops);
}

vector <pair<string_view, string_view>>
AdjacentStops(const renderer_utils::Buses &buses) {
  vector<pair<string_view, string_view>> result;
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

vector <vector<string_view>> CompressNonadjacent(
    const vector <string_view> &stops,
    const vector <pair<string_view,
                       string_view>> &neighbors) {
  unordered_map<string_view, int> stop_to_layer;
  for (int i = 0; i < stops.size(); ++i)
    stop_to_layer[stops[i]] = i;

  vector<int> max_next(stops.size(), static_cast<int>(stops.size()) - 1);

  for (auto &[prev, curr] : neighbors) {
    auto prev_layer = stop_to_layer[prev];
    auto curr_layer = stop_to_layer[curr];
    if (prev_layer > curr_layer) swap(prev_layer, curr_layer);
    max_next[prev_layer] = min(max_next[prev_layer], curr_layer - 1);
  }

  vector<vector<string_view>> res;
  int last = -1;
  for (int layer = 0; layer < stops.size(); ++layer) {
    if (layer > last) {
      res.push_back({stops[layer]});
      last = max_next[layer];
      continue;
    }
    last = min(last, max_next[layer]);
    res.back().push_back(stops[layer]);
  }
  return res;
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
