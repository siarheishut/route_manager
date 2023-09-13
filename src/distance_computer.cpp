#include "distance_computer.h"

#include <string>
#include <vector>

#include "common.h"

namespace rm {
double ComputeGeoDistance(const std::vector<std::string> &stops,
                          const StopDict &dict) {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    distance += sphere::CalculateDistance(dict.at(stops[i - 1]).coords,
                                          dict.at(stops[i]).coords);
  }
  return distance;
}

double ComputeRoadDistance(const std::vector<std::string> &stops,
                           const StopDict &dict) {
  double distance = 0;
  for (int i = 1; i < stops.size(); ++i) {
    auto &dists = dict.at(stops[i - 1]).dists;

    if (auto found = dists.find(stops[i]); found != dists.end())
      distance += found->second;
    else
      distance += sphere::CalculateDistance(dict.at(stops[i - 1]).coords,
                                            dict.at(stops[i]).coords);
  }
  return distance;
}
}
