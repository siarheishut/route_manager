#ifndef ROOT_MANAGER_SRC_BUS_MANAGER_H_
#define ROOT_MANAGER_SRC_BUS_MANAGER_H_

#include "request_types.h"
#include "common.h"

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

using RouteResponse = RouteInfo;

class BusManager {
 public:
  static std::unique_ptr<BusManager> Create(std::vector<PostRequest> requests);

  std::optional<BusResponse> GetBusInfo(const std::string &bus) const;

  std::optional<StopResponse> GetStopInfo(const std::string &stop) const;

 private:
  explicit BusManager(std::vector<PostRequest> requests);

  void AddStop(const std::string &stop, sphere::Coords coords,
               const std::map<std::string, int> &stops);

  void AddBus(std::string bus, std::vector<std::string> stops);

 private:
  StopDict stop_info_;
  BusDict bus_info_;
};
}

#endif // ROOT_MANAGER_SRC_BUS_MANAGER_H_
