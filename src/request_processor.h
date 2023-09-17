#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include <memory>
#include <ostream>
#include <vector>

#include "json.h"

#include "bus_manager.h"
#include "request_types.h"

namespace rm {
json::Dict ToJson(std::optional<BusResponse> resp, int id);
json::Dict ToJson(std::optional<StopResponse> resp, int id);
json::List ToJson(std::vector<RouteResponse::Item> response_items);
json::Dict ToJson(std::optional<RouteResponse> resp, int id);

class Processor {
 public:
  static std::unique_ptr<Processor> Create(
      std::vector<PostRequest> requests,
      const RoutingSettings &routing_settings);

  json::List Process(const std::vector<GetRequest> &requests) const;

 private:
  explicit Processor(std::unique_ptr<BusManager> bus_manager);

  json::Dict Process(const GetBusRequest &request) const;
  json::Dict Process(const GetStopRequest &request) const;
  json::Dict Process(const GetRouteRequest &request) const;
  json::Dict Process(const GetMapRequest &request) const;

  std::unique_ptr<BusManager> bus_manager_;
};
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
