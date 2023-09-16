#ifndef ROOT_MANAGER_SRC_REQUEST_PARSER_H_
#define ROOT_MANAGER_SRC_REQUEST_PARSER_H_

#include <optional>
#include <vector>

#include "json.h"

#include "request_types.h"

namespace rm {
std::optional<RoutingSettings> ParseRoutingSettings(json::Dict settings);

std::optional<std::vector<PostRequest>> ParseInput(json::List base_requests);
std::optional<std::vector<GetRequest>> ParseOutput(json::List stat_requests);

std::optional<PostRequest> ParseInputRequest(json::Dict request_data);
std::optional<GetRequest> ParseOutputRequest(json::Dict request_data);

std::optional<PostStopRequest> ParsePostStopRequest(json::Dict request_data);
std::optional<PostBusRequest> ParsePostBusRequest(json::Dict request_data);

std::optional<GetStopRequest> ParseGetStopRequest(json::Dict request_data);
std::optional<GetBusRequest> ParseGetBusRequest(json::Dict request_data);
std::optional<GetRouteRequest> ParseGetRouteRequest(json::Dict request_data);
std::optional<GetMapRequest> ParseGetMapRequest(json::Dict request_data);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PARSER_H_
