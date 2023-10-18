#include "test_utils.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "svg/common.h"

#include "src/map_renderer_utils.h"

using namespace std;

namespace rm {
bool CompareLength(double lhs, double rhs, int precision) {
  stringstream ssl, ssr;
  ssl << setprecision(precision) << lhs;
  ssr << setprecision(precision) << rhs;
  return ssl.str() == ssr.str();
}

bool operator==(const GetBusRequest &lhs, const GetBusRequest &rhs) {
  return lhs.bus == rhs.bus;
}

bool operator==(const GetStopRequest &lhs, const GetStopRequest &rhs) {
  return lhs.stop == rhs.stop;
}

bool operator==(const GetRouteRequest &lhs, const GetRouteRequest &rhs) {
  return tie(lhs.from, lhs.to, lhs.id) == tie(rhs.from, rhs.to, rhs.id);
}

bool operator==(const GetMapRequest &lhs, const GetMapRequest &rhs) {
  return lhs.id == rhs.id;
}

bool operator==(const BusResponse &lhs, const BusResponse &rhs) {
  return tie(lhs.stop_count, lhs.unique_stop_count, lhs.length)
      == tie(rhs.stop_count, rhs.unique_stop_count, rhs.length);
}

bool operator==(const PostBusRequest &lhs, const PostBusRequest &rhs) {
  return tie(lhs.bus, lhs.stops, lhs.is_roundtrip)
      == tie(rhs.bus, rhs.stops, rhs.is_roundtrip);
}

bool operator==(const PostStopRequest &lhs, const PostStopRequest &rhs) {
  return tie(lhs.stop,
             lhs.coords.latitude,
             lhs.coords.longitude,
             lhs.stop_distances)
      == tie(rhs.stop,
             rhs.coords.latitude,
             rhs.coords.longitude,
             rhs.stop_distances);
}

bool operator==(const RoutingSettings &lhs, const RoutingSettings &rhs) {
  return tie(lhs.bus_wait_time, lhs.bus_velocity)
      == tie(rhs.bus_wait_time, rhs.bus_velocity);
}

bool operator==(const RenderingSettings &lhs, const RenderingSettings &rhs) {
  using namespace svg;

  return tuple(lhs.frame,
               lhs.stop_radius,
               lhs.line_width,
               lhs.underlayer_width,
               lhs.underlayer_color,
               lhs.stop_label_font_size,
               lhs.stop_label_offset,
               lhs.color_palette.size(),
               lhs.layers,
               lhs.color_palette) ==
      tuple(rhs.frame,
            rhs.stop_radius,
            rhs.line_width,
            rhs.underlayer_width,
            rhs.underlayer_color,
            rhs.stop_label_font_size,
            rhs.stop_label_offset,
            rhs.color_palette.size(),
            rhs.layers,
            rhs.color_palette);
}

bool operator==(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs) {
  return tie(lhs.stop, lhs.time) == tie(rhs.stop, rhs.time);
}

bool operator==(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs) {
  if (!CompareLength(lhs.time, rhs.time, 9)) return false;
  return tie(lhs.bus, lhs.span_count) == tie(rhs.bus, rhs.span_count);
}

bool operator==(const RouteResponse &lhs, const RouteResponse &rhs) {
  return tie(lhs.items, lhs.time) == tie(rhs.items, rhs.time);
}

bool operator!=(const GetBusRequest &lhs, const GetBusRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const GetStopRequest &lhs, const GetStopRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const GetRouteRequest &lhs, const GetRouteRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const GetMapRequest &lhs, const GetMapRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const BusResponse &lhs, const BusResponse &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const PostBusRequest &lhs, const PostBusRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const PostStopRequest &lhs, const PostStopRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const RoutingSettings &lhs, const RoutingSettings &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const RenderingSettings &lhs, const RenderingSettings &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs) {
  return !(lhs == rhs);
}
bool operator!=(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const RouteResponse &lhs, const RouteResponse &rhs) {
  return !(lhs == rhs);
}

template<typename T>
ostream &operator<<(ostream &out, const vector <T> &str_v) {
  for (auto &item : str_v)
    out << ' ' << item;
  return out;
}

ostream &operator<<(ostream &out, const PostBusRequest &br) {
  return out << br.bus << ": " << br.stops << ' ' << br.is_roundtrip;
}

ostream &operator<<(ostream &out, const PostStopRequest &sr) {
  out << sr.stop << ": " << sr.coords.latitude << ' ' <<
      sr.coords.longitude << '\t';
  for (auto [stop, dist] : sr.stop_distances) {
    out << '{' << stop << ':' << dist << "} ";
  }
  return out;
}

ostream &operator<<(ostream &out, const GetBusRequest &br) {
  return out << br.bus << " – " << br.id;
}

ostream &operator<<(ostream &out, const GetStopRequest &br) {
  return out << br.stop << " – " << br.id;
}

ostream &operator<<(ostream &out, const GetMapRequest &mr) {
  return out << mr.id;
}

ostream &operator<<(ostream &out, const BusResponse &br) {
  return out << br.stop_count << ' ' << br.unique_stop_count << ' '
             << br.length;
}

ostream &operator<<(ostream &out, const RoutingSettings &settings) {
  return out << settings.bus_wait_time << ' ' << settings.bus_velocity;
}

ostream &operator<<(ostream &out, const RenderingSettings &settings) {
  return out << settings.frame.width << ' ' << settings.frame.height << ' ' <<
             settings.frame.padding << ' ' << settings.stop_radius << ' ' <<
             settings.line_width << ' ' << settings.stop_label_font_size
             << ' ' << settings.stop_label_offset << ' ' <<
             settings.underlayer_color << ' ' << settings.underlayer_width
             << ' ' << settings.color_palette;
}

ostream &operator<<(ostream &out, const RouteResponse::RoadItem &ri) {
  return out << ri.bus << '-' << ri.time << '-' << ri.span_count;
}

ostream &operator<<(ostream &out, const RouteResponse::WaitItem &wi) {
  return out << wi.stop << '-' << wi.time;
}

ostream &operator<<(ostream &out, const RouteResponse::Item &item) {
  visit([&](auto &&var) {
    out << var;
  }, item);
  return out;
}

ostream &operator<<(ostream &out, const RouteResponse &rr) {
  return out << rr.time << ' ' << rr.items;
}
}

namespace rm::renderer_utils {
bool operator==(const Frame &lhs, const Frame &rhs) {
  return tie(lhs.width, lhs.height, lhs.padding) ==
      tie(rhs.width, rhs.height, rhs.padding);
}

bool operator!=(const Frame &lhs, const Frame &rhs) {
  return !(lhs == rhs);
}

ostream &operator<<(ostream &out, const Frame &frame) {
  return out << frame.width << ' ' << frame.height << ' ' << frame.padding;
}
}

namespace svg {
bool operator==(Point lhs, Point rhs) {
  return tie(lhs.x, lhs.y) == tie(rhs.x, rhs.y);
}

bool operator!=(Point lhs, Point rhs) {
  return !(lhs == rhs);
}

bool operator==(Rgb lhs, Rgb rhs) {
  return tuple<int, int, int>(lhs.red, lhs.green, lhs.blue)
      == tuple<int, int, int>(rhs.red, rhs.green, rhs.blue);
}

bool operator!=(Rgb lhs, Rgb rhs) {
  return !(lhs == rhs);
}

bool operator==(Rgba lhs, Rgba rhs) {
  return
      tuple<int, int, int, double>(lhs.red, lhs.green, lhs.blue, lhs.alpha) ==
          tuple<int, int, int, double>(rhs.red, rhs.green, rhs.blue, rhs.alpha);
}

bool operator!=(Rgba lhs, Rgba rhs) {
  return !(lhs == rhs);
}

bool operator==(const Color &lhs, const Color &rhs) {
  if (holds_alternative<string>(lhs) && holds_alternative<string>(rhs)) {
    return get<string>(lhs) == get<string>(rhs);
  }
  if (holds_alternative<Rgb>(lhs) && holds_alternative<Rgb>(rhs)) {
    return get<Rgb>(lhs) == get<Rgb>(rhs);
  }
  if (holds_alternative<Rgba>(lhs) && holds_alternative<Rgba>(rhs)) {
    return get<Rgba>(lhs) == get<Rgba>(rhs);
  }
  return holds_alternative<monostate>(lhs) && holds_alternative<monostate>(rhs);
}

bool operator!=(const Color &lhs, const Color &rhs) {
  return !(lhs == rhs);
}

ostream &operator<<(ostream &out, const Point point) {
  return out << '{' << point.x << ',' << point.y << '}';
}
}

namespace rm::sphere {
bool operator==(const Coords &lhs, const Coords &rhs) {
  return std::tie(lhs.longitude, lhs.latitude) ==
      std::tie(rhs.longitude, rhs.latitude);
}

bool operator!=(const Coords &lhs, const Coords &rhs) {
  return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &out, const Coords &coords) {
  return out << "{lon = " << coords.longitude << ", lat = " << coords.latitude
             << '}';
}
}
