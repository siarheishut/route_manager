#ifndef ROOT_MANAGER_SRC_REQUEST_PARSER_H_
#define ROOT_MANAGER_SRC_REQUEST_PARSER_H_

#include <optional>
#include <vector>

#include "json.h"
#include "svg/common.h"

#include "request_types.h"

namespace rm {
std::optional<utils::SerializationSettings> ParseSerializationSettings(
    json::Dict settings);

std::optional<utils::RoutingSettings> ParseRoutingSettings(json::Dict settings);
std::optional<utils::RenderingSettings> ParseRenderingSettings(
    json::Dict settings);

std::optional<std::vector<utils::PostRequest>> ParseInput(
    json::List base_requests);
std::optional<std::vector<utils::GetRequest>> ParseOutput(
    json::List stat_requests);

std::optional<utils::PostRequest> ParseInputRequest(json::Dict request_data);
std::optional<utils::GetRequest> ParseOutputRequest(json::Dict request_data);

std::optional<utils::PostStopRequest> ParsePostStopRequest(
    json::Dict request_data);
std::optional<utils::PostBusRequest> ParsePostBusRequest(
    json::Dict request_data);

std::optional<utils::GetStopRequest> ParseGetStopRequest(
    json::Dict request_data);
std::optional<utils::GetBusRequest> ParseGetBusRequest(json::Dict request_data);
std::optional<utils::GetRouteRequest> ParseGetRouteRequest(
    json::Dict request_data);
std::optional<utils::GetMapRequest> ParseGetMapRequest(json::Dict request_data);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PARSER_H_
