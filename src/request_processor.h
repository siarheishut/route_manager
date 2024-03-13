#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include <memory>
#include <ostream>
#include <vector>

#include "json.h"

#include "bus_manager.h"
#include "map_renderer.h"
#include "request_types.h"
#include "response_types.h"

namespace rm {
json::Dict ToJson(std::optional<utils::BusResponse> resp, int id);
json::Dict ToJson(std::optional<utils::StopResponse> resp, int id);
json::List ToJson(std::vector<utils::RouteResponse::Item> response_items);
json::Dict ToJson(std::optional<utils::RouteResponse> response,
                  std::optional<std::string> map, int id);
json::Dict ToJson(utils::MapResponse resp, int id);

class Processor {
 public:
  static std::unique_ptr<Processor> Deserialize(
      const ::TransportCatalog::TransportCatalog &proto_catalog);

  static std::unique_ptr<Processor> Create(
      std::vector<utils::PostRequest> requests,
      const utils::RoutingSettings &routing_settings,
      const utils::RenderingSettings &rendering_settings);

  json::List Process(const std::vector<utils::GetRequest> &requests) const;

  ::TransportCatalog::TransportCatalog Serialize() const;

 private:
  Processor(std::shared_ptr<TransportCatalog> catalog,
            std::shared_ptr<RouteManager> route_manager,
            std::unique_ptr<BusManager> bus_manager,
            std::unique_ptr<MapRenderer> map_renderer,
            utils::RenderingSettings rendering_settings);

  json::Dict Process(const utils::GetBusRequest &request) const;
  json::Dict Process(const utils::GetStopRequest &request) const;
  json::Dict Process(const utils::GetRouteRequest &request) const;
  json::Dict Process(const utils::GetMapRequest &request) const;

  std::shared_ptr<TransportCatalog> catalog_;
  std::shared_ptr<RouteManager> route_manager_;
  std::unique_ptr<BusManager> bus_manager_;
  std::unique_ptr<MapRenderer> map_renderer_;
  rm::utils::RenderingSettings rendering_settings_;
};
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
