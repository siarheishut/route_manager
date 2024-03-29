#include "sphere.h"

#include <cmath>

namespace rm::sphere {
// Used the solution from https://stackoverflow.com/a/70429229.
// Uses Haversine formula https://en.wikipedia.org/wiki/Haversine_formula
// with asin expressed with atan2 for better accuracy.
double CalculateDistance(Coords lhs, Coords rhs) {
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
}
