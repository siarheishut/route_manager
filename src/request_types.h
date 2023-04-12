#ifndef ROOT_MANAGER_SRC_REQUEST_TYPES_H_
#define ROOT_MANAGER_SRC_REQUEST_TYPES_H_

#include <string>
#include <variant>
#include <vector>

namespace rm {
struct GetBusRequest {
  std::string bus;
};

struct PostBusRequest {
  std::string bus;
  std::vector<std::string> stops;
};

struct Coords {
  double latitude, longitude;
};

struct PostStopRequest {
  std::string stop;
  Coords coords;
};

using PostRequest = std::variant<PostBusRequest, PostStopRequest>;
using GetRequest = std::variant<GetBusRequest>;
}

#endif // ROOT_MANAGER_SRC_REQUEST_TYPES_H_
