#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include "request_types.h"

#include <string>
#include <optional>
#include <unordered_map>
#include <map>
#include <vector>

namespace rm {
struct BusResponse {
  int stop_count;
  int unique_stop_count;
  double length;
  double curvature;
};

struct StopResponse {
  std::vector<std::string> buses;
};

struct StopInfo {
  // Distances to other stops.
  std::unordered_map<std::string, int> dists;
  Coords coords;
  std::vector<std::string> buses;
};

class BusManager {
 public:
  static std::unique_ptr<BusManager> Create(const std::vector<PostRequest> &requests);

  std::optional<BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<StopResponse> GetStopInfo(const std::string &stop) const;

 private:
  explicit BusManager(std::vector<PostRequest> requests);

  void AddStop(const std::string &stop, Coords coords,
               std::map<std::string, int> stops);

  void AddBus(std::string bus, std::vector<std::string> stops);

 private:
  struct BusInfo {
    std::vector<std::string> stops;
    int unique_stop_count;
    double distance;
    double curvature;
  };
  double ComputeGeoDistance(const std::vector<std::string> &stops) const;
  double ComputeRoadDistance(const std::vector<std::string> &stops) const;

  std::unordered_map<std::string, StopInfo> stop_info_;
  std::unordered_map<std::string, BusInfo> bus_info_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
