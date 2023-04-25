#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include "bus_manager.h"
#include "request_types.h"
#include "json/json.h"

#include <ostream>

namespace rm {
rm::json::Dict ProcessBusRequest(const BusManager &bm,
                                 const GetBusRequest &request);

rm::json::Dict ProcessStopRequest(const BusManager &bm,
                                  const GetStopRequest &request);

rm::json::List ProcessRequests(const BusManager &bm,
                               const std::vector<GetRequest> &request);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
