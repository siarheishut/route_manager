#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include "bus_manager.h"
#include "request_types.h"
#include "lib/json/include/json.h"

#include <ostream>

namespace rm {
json::Dict ProcessRouteRequest(const BusManager &bm,
                               const GetRouteRequest &request);

json::Dict ProcessBusRequest(const BusManager &bm,
                             const GetBusRequest &request);

json::Dict ProcessStopRequest(const BusManager &bm,
                              const GetStopRequest &request);

json::List ProcessRequests(const BusManager &bm,
                           std::vector<GetRequest> request);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
