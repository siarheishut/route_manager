#ifndef ROOT_MANAGER_SRC_REQUEST_READER_H_
#define ROOT_MANAGER_SRC_REQUEST_READER_H_

#include "src/request_types.h"

#include <istream>
#include <optional>
#include <string_view>
#include <vector>

namespace rm {
std::optional<std::vector<PostRequest>> ReadInputRequests(std::istream &in);

std::optional<std::vector<GetRequest>> ReadOutputRequests(std::istream &in);

std::optional<PostBusRequest> ParsePostBusRequest(std::string_view sv);

std::optional<PostStopRequest> ParsePostStopRequest(std::string_view sv);

std::optional<GetBusRequest> ParseGetBusRequest(std::string_view sv);

std::optional<GetStopRequest> ParseGetStopRequest(std::string_view sv);
}

#endif // ROOT_MANAGER_SRC_REQUEST_READER_H_
