#include "bus_manager.h"

#include <iomanip>
#include <ostream>

namespace rm {
void ProcessBusRequest(const BusManager &bm,
                       const GetBusRequest &request,
                       std::ostream &os) {
  auto resp = bm.GetBusInfo(request.bus);

  if (!resp) {
    os << "Bus " << request.bus << ": not found\n";
    return;
  }

  os << "Bus " << request.bus << ": " << resp->stop_count
     << " stops on route, " << resp->unique_stop_count
     << " unique stops, " << std::setprecision(6)
     << resp->length << " route length\n";
}

void ProcessStopRequest(const BusManager &bm,
                        const GetStopRequest &request,
                        std::ostream &os) {
  auto resp = bm.GetStopInfo(request.stop);

  if (!resp)
    os << "Stop " << request.stop << ": not found\n";
  else if (resp->buses.empty())
    os << "Stop " << request.stop << ": no buses\n";
  else {
    os << "Stop " << request.stop << ": buses";
    for (auto &bus : resp->buses) {
      os << ' ' << bus;
    }
    os << '\n';
  }
}

void ProcessRequest(const BusManager &bm,
                    const GetRequest &request,
                    std::ostream &os) {
  if (auto ptr_b = std::get_if<GetBusRequest>(&request))
    ProcessBusRequest(bm, *ptr_b, os);
  else if (auto ptr_s = std::get_if<GetStopRequest>(&request))
    ProcessStopRequest(bm, *ptr_s, os);
}
}
