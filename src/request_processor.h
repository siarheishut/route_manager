#ifndef ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
#define ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_

#include "bus_manager.h"

#include <ostream>

namespace rm {
void ProcessBusRequest(const BusManager &bm,
                       const GetBusRequest &request,
                       std::ostream &os);

void ProcessStopRequest(const BusManager &bm,
                        const GetStopRequest &request,
                        std::ostream &os);

void ProcessRequest(const BusManager &bm,
                    const GetRequest &request,
                    std::ostream &os);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PROCESSOR_H_
