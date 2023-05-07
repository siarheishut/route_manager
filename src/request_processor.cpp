#include "request_processor.h"
#include "bus_manager.h"

#include <variant>

namespace rm {
json::Dict ProcessBusRequest(const BusManager &bm,
                             const GetBusRequest &request) {
  json::Dict response;

  auto bm_resp = bm.GetBusInfo(request.bus);

  if (!bm_resp) {
    response.emplace("request_id", request.id);
    response.emplace("error_message", "not found");
    return response;
  }
  response.emplace("route_length", bm_resp->length);
  response.emplace("request_id", request.id);
  response.emplace("curvature", bm_resp->curvature);
  response.emplace("stop_count", bm_resp->stop_count);
  response.emplace("unique_stop_count", bm_resp->unique_stop_count);

  return response;
}

json::Dict ProcessStopRequest(const BusManager &bm,
                              const GetStopRequest &request) {
  json::Dict response;

  auto bm_resp = bm.GetStopInfo(request.stop);

  response.emplace("request_id", request.id);
  if (!bm_resp) {
    response.emplace("error_message", "not found");
  } else {
    response.emplace("buses",
                     json::List(std::move_iterator(bm_resp->buses.begin()),
                                std::move_iterator(bm_resp->buses.end())));
  }

  return response;
}

json::List ProcessRequests(const BusManager &bm,
                           const std::vector<GetRequest> &requests) {
  json::List node;

  for (auto &req : requests) {
    if (auto ptr_b = std::get_if<GetBusRequest>(&req))
      node.emplace_back(ProcessBusRequest(bm, *ptr_b));
    else if (auto ptr_s = std::get_if<GetStopRequest>(&req))
      node.emplace_back(ProcessStopRequest(bm, *ptr_s));
  }

  return node;
}
}
