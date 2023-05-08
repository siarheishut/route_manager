#ifndef ROOT_MANAGER_SRC_REQUEST_PARSER_H_
#define ROOT_MANAGER_SRC_REQUEST_PARSER_H_

#include "lib/json/include/json.h"
#include "request_types.h"

#include <optional>
#include <vector>

namespace rm {
std::optional<RoutingSettings> ParseSettings(json::Dict settings);

std::optional<std::vector<PostRequest>> ParseInput(json::List base_requests);
std::optional<std::vector<GetRequest>> ParseOutput(json::List stat_requests);

std::optional<PostRequest> ParseInputRequest(json::Dict dict);
std::optional<GetRequest> ParseOutputRequest(json::Dict dict);

std::optional<PostStopRequest> ParsePostStopRequest(json::Dict dict);
std::optional<PostBusRequest> ParsePostBusRequest(json::Dict dict);

std::optional<GetRouteRequest> ParseGetRouteRequest(json::Dict dict);
std::optional<GetStopRequest> ParseGetStopRequest(json::Dict dict);
std::optional<GetBusRequest> ParseGetBusRequest(json::Dict dict);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PARSER_H_
