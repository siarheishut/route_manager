#include "bus_manager.h"

#include <iomanip>
#include <ostream>
#include <variant>

namespace rm {
void ProcessRequest(const BusManager &bm,
                    const GetRequest &request,
                    std::ostream &os) {
  if (auto ptr_b = std::get_if<GetBusRequest>(&request)) {
    auto resp = bm.GetBusInfo(ptr_b->bus);

    if (!resp) {
      os << "Bus " << ptr_b->bus << ": not found\n";
      return;
    }

    os << "Bus " << ptr_b->bus << ": " << resp->stop_count
       << " stops on route, " << resp->unique_stop_count
       << " unique stops, " << std::setprecision(6)
       << resp->length << " route length\n";
  }
}
}
