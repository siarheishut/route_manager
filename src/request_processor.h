#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include "bus_manager.h"
#include "request_types.h"
#include "json.h"

#include <ostream>

namespace rm {
json::Dict ToJson(std::optional<BusResponse> resp, int id);
json::Dict ToJson(std::optional<StopResponse> resp, int id);

json::Dict Process(const BusManager &bm, const GetBusRequest &request);
json::Dict Process(const BusManager &bm, const GetStopRequest &request);
json::Dict Process(const BusManager &bm, const GetRouteRequest &request);

json::List ProcessRequests(const BusManager &bm,
                           std::vector<GetRequest> requests);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
