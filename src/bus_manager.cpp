#include "bus_manager.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace {
// Used the solution from https://stackoverflow.com/a/70429229.
// Uses Haversine formula https://en.wikipedia.org/wiki/Haversine_formula
// with asin expressed with atan2 for better accuracy.
double CalculateDistance(rm::Coords lhs, rm::Coords rhs) {
  constexpr double earth_radius_m = 6371.0 * 1000.0;

  const double d_lat = rhs.latitude - lhs.latitude;
  const double d_long = rhs.longitude - lhs.longitude;

  double a =
      std::sin(d_lat / 2) * std::sin(d_lat / 2) + std::sin(d_long / 2) *
          std::sin(d_long / 2) *
          std::cos(lhs.latitude) *
          std::cos(rhs.latitude);
  double c = earth_radius_m * 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

  return c;
}

int ComputeUniqueCount(std::vector<std::string> stops) {
  std::sort(begin(stops), end(stops));
  return std::distance(stops.begin(),
                       std::unique(begin(stops), end(stops)));
}
}

namespace rm {
BusManager::BusManager(std::vector<PostRequest> requests) {
  for (auto &request : requests) {
    if (auto ptr_b = std::get_if<PostBusRequest>(&request)) {
      AddBus(ptr_b->bus, ptr_b->stops);
    } else if (auto ptr_s = std::get_if<PostStopRequest>(&request)) {
      AddStop(ptr_s->stop,
              ptr_s->coords);
    }
  }

  for (auto &[bus, bus_info] : bus_info_) {
    bus_info.distance = ComputeDistance(bus_info.stops);
    bus_info.unique_stop_count = ComputeUniqueCount(bus_info.stops);
  }
}

void BusManager::AddStop(const std::string &stop,
                         Coords coords) {
  constexpr double k = 3.1415926535 / 180;
  stop_coords_.emplace(stop, Coords{coords.latitude * k, coords.longitude * k});
}

void BusManager::AddBus(const std::string &bus,
                        std::vector<std::string> stops) {
  BusInfo bus_info;
  bus_info.stops = std::move(stops);
  bus_info_[bus] = bus_info;
}

std::optional<BusResponse> BusManager::GetBusInfo(const std::string &bus) const {
  auto it = bus_info_.find(bus);
  if (it == bus_info_.end()) return std::nullopt;

  auto &b = it->second;
  return BusResponse{
      .stop_count = static_cast<int>(b.stops.size()),
      .unique_stop_count = b.unique_stop_count,
      .length = b.distance
  };
}

double BusManager::ComputeDistance(const std::vector<std::string> &stops) {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    distance += CalculateDistance(stop_coords_[stops[i - 1]],
                                  stop_coords_[stops[i]]);

  }
  return distance;
}
}
