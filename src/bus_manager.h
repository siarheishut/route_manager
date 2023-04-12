#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include "request_types.h"

#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

namespace rm {
struct BusResponse {
  int stop_count;
  int unique_stop_count;
  double length;
};

class BusManager {
 public:
  explicit BusManager(std::vector<PostRequest> requests);

  std::optional<BusResponse> GetBusInfo(const std::string &bus) const;

 private:
  void AddStop(const std::string &stop,
               Coords coords);

  void AddBus(const std::string &bus,
              std::vector<std::string> stops);

 private:
  struct BusInfo {
    std::vector<std::string> stops;
    int unique_stop_count;
    double distance;
  };
  double ComputeDistance(const std::vector<std::string> &stops);

  std::unordered_map<std::string, Coords> stop_coords_;
  std::unordered_map<std::string, BusInfo> bus_info_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
