#ifndef ROOT_MANAGER_SRC_REQUEST_PARSER_H_
#define ROOT_MANAGER_SRC_REQUEST_PARSER_H_

#include "json/json.h"
#include "request_types.h"

#include <optional>
#include <vector>

namespace rm {
std::optional<std::vector<PostRequest>> ParseInput(json::List base_requests);
std::optional<std::vector<GetRequest>> ParseOutput(json::List stat_requests);

std::optional<PostRequest> ParseInputRequest(json::Dict request_data);
std::optional<GetRequest> ParseOutputRequest(json::Dict request_data);

std::optional<PostStopRequest> ParsePostStopRequest(json::Dict request_data);
std::optional<PostBusRequest> ParsePostBusRequest(json::Dict request_data);

std::optional<GetStopRequest> ParseGetStopRequest(json::Dict request_data);
std::optional<GetBusRequest> ParseGetBusRequest(json::Dict request_data);
}

#endif // ROOT_MANAGER_SRC_REQUEST_PARSER_H_
