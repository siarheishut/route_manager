#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include <memory>
#include <ostream>
#include <vector>

#include "json.h"

#include "bus_manager.h"
#include "map_renderer.h"
#include "request_types.h"

namespace rm {
struct MapResponse {
  std::string map;
};

json::Dict ToJson(std::optional<BusResponse> resp, int id);
json::Dict ToJson(std::optional<StopResponse> resp, int id);
json::List ToJson(std::vector<RouteResponse::Item> response_items);
json::Dict ToJson(std::optional<RouteResponse> resp, std::string map, int id);
json::Dict ToJson(MapResponse resp, int id);

class Processor {
 public:
  static std::unique_ptr<Processor> Create(
      std::vector<PostRequest> requests,
      const RoutingSettings &routing_settings,
      const RenderingSettings &rendering_settings);

  json::List Process(const std::vector<GetRequest> &requests) const;

 private:
  Processor(std::unique_ptr<BusManager> bus_manager,
            std::unique_ptr<MapRenderer> map_renderer);

  json::Dict Process(const GetBusRequest &request) const;
  json::Dict Process(const GetStopRequest &request) const;
  json::Dict Process(const GetRouteRequest &request) const;
  json::Dict Process(const GetMapRequest &request) const;

  std::unique_ptr<BusManager> bus_manager_;
  std::unique_ptr<MapRenderer> map_renderer_;
};
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
