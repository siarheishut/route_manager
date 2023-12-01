#ifndef ROOT_MANAGER_SRC_DISTANCE_COMPUTER_H_
#define ROOT_MANAGER_SRC_DISTANCE_COMPUTER_H_

#include <string>
#include <vector>

#include "common.h"

namespace rm {
double ComputeGeoDistance(const std::vector<std::string> &stops,
                          const utils::StopDict &dict);

double ComputeRoadDistance(const std::vector<std::string> &stops,
                           const utils::StopDict &dict);
}

#endif // ROOT_MANAGER_SRC_DISTANCE_COMPUTER_H_
