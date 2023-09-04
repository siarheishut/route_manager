#ifndef ROOT_MANAGER_SRC_DISTANCE_COMPUTER_H_
#define ROOT_MANAGER_SRC_DISTANCE_COMPUTER_H_

#include "common.h"

#include <string>
#include <vector>

namespace rm {
double ComputeGeoDistance(const std::vector<std::string> &stops,
                          const StopDict &dict);

double ComputeRoadDistance(const std::vector<std::string> &stops,
                           const StopDict &dict);
}

#endif // ROOT_MANAGER_SRC_DISTANCE_COMPUTER_H_
