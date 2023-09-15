#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include <ostream>

#include "json.h"

#include "bus_manager.h"
#include "request_types.h"

namespace rm {
json::Dict ToJson(std::optional<BusResponse> resp, int id);
json::Dict ToJson(std::optional<StopResponse> resp, int id);
json::List ToJson(std::vector<RouteResponse::Item> response_items);
json::Dict ToJson(std::optional<RouteResponse> resp, int id);
json::Dict ToJson(MapResponse resp, int id);

json::Dict Process(const BusManager &bm, const GetBusRequest &request);
json::Dict Process(const BusManager &bm, const GetStopRequest &request);
json::Dict Process(const BusManager &bm, const GetRouteRequest &request);
json::Dict Process(const BusManager &bm, const GetMapRequest &request);

json::List ProcessRequests(const BusManager &bm,
                           std::vector<GetRequest> requests);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
