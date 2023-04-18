#ifndef ROOT_MANAGER_SRC_REQUEST_TYPES_H_
#define ROOT_MANAGER_SRC_REQUEST_TYPES_H_

#include <string>
#include <variant>
#include <vector>

namespace rm {
struct GetBusRequest {
  std::string bus;
};

struct GetStopRequest {
  std::string stop;
};

struct PostBusRequest {
  std::string bus;
  std::vector<std::string> stops;
};

struct Coords {
  double latitude, longitude;
};

struct RoadDistance {
  std::string destination;
  int distance;
};

struct PostStopRequest {
  std::string stop;
  Coords coords;
  // Pairs of stop_to and dist from stop to stop_to.
  std::vector<RoadDistance> stops;
};

using PostRequest = std::variant<PostBusRequest, PostStopRequest>;
using GetRequest = std::variant<GetBusRequest, GetStopRequest>;
}

#endif // ROOT_MANAGER_SRC_REQUEST_TYPES_H_
