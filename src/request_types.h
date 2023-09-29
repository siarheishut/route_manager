#ifndef ROOT_MANAGER_SRC_REQUEST_TYPES_H_
#define ROOT_MANAGER_SRC_REQUEST_TYPES_H_

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "sphere.h"

#include "svg/common.h"

namespace rm {
struct RoutingSettings {
  // measured in minutes.
  int bus_wait_time;
  // measured in km/h.
  double bus_velocity;
};

struct RenderingSettings {
  double width;
  double height;
  double padding;
  double stop_radius;
  double line_width;
  int stop_label_font_size;
  svg::Point stop_label_offset;
  svg::Color underlayer_color;
  double underlayer_width;
  std::vector<svg::Color> color_palette;
  int bus_label_font_size;
  svg::Point bus_label_offset;
  std::vector<std::string> layers;
};

struct GetBusRequest {
  int id;
  std::string bus;
};

struct GetStopRequest {
  int id;
  std::string stop;
};

struct GetRouteRequest {
  int id;
  std::string from;
  std::string to;
};

struct GetMapRequest {
  int id;
};

struct PostBusRequest {
  std::string bus;
  std::vector<std::string> stops;
  bool is_roundtrip;
};

struct PostStopRequest {
  std::string stop;
  sphere::Coords coords;
  // Pairs of stop_to and dist from stop to stop_to.
  std::map<std::string, int> stop_distances;
};

using PostRequest = std::variant<PostBusRequest, PostStopRequest>;
using GetRequest = std::variant<GetBusRequest, GetStopRequest, GetRouteRequest,
                                GetMapRequest>;
}

#endif // ROOT_MANAGER_SRC_REQUEST_TYPES_H_
