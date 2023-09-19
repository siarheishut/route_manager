#include "test_utils.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "svg/common.h"

namespace rm {
bool CompareLength(double lhs, double rhs, int precision) {
  std::stringstream ssl, ssr;
  ssl << std::setprecision(precision) << lhs;
  ssr << std::setprecision(precision) << rhs;
  return ssl.str() == ssr.str();
}

bool operator==(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs) {
  return lhs.bus == rhs.bus;
}

bool operator==(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs) {
  return lhs.stop == rhs.stop;
}

bool operator==(const rm::GetRouteRequest &lhs,
                const rm::GetRouteRequest &rhs) {
  return std::tie(lhs.from, lhs.to, lhs.id)
      == std::tie(rhs.from, rhs.to, rhs.id);
}

bool operator==(const rm::GetMapRequest &lhs, const rm::GetMapRequest &rhs) {
  return lhs.id == rhs.id;
}

bool operator==(const rm::BusResponse &lhs, const rm::BusResponse &rhs) {
  return std::tie(lhs.stop_count, lhs.unique_stop_count, lhs.length)
      == std::tie(rhs.stop_count, rhs.unique_stop_count, rhs.length);
}

bool operator==(const rm::PostBusRequest &lhs, const rm::PostBusRequest &rhs) {
  return std::tie(lhs.bus, lhs.stops) == std::tie(rhs.bus, rhs.stops);
}

bool operator==(const rm::PostStopRequest &lhs,
                const rm::PostStopRequest &rhs) {
  return
      std::tie(lhs.stop,
               lhs.coords.latitude,
               lhs.coords.longitude,
               lhs.stop_distances)
          == std::tie(rhs.stop, rhs.coords.latitude, rhs.coords.longitude,
                      rhs.stop_distances);
}

bool operator==(const rm::RoutingSettings &lhs,
                const rm::RoutingSettings &rhs) {
  return std::tie(lhs.bus_wait_time, lhs.bus_velocity)
      == std::tie(rhs.bus_wait_time, rhs.bus_velocity);
}

bool operator==(svg::Point lhs, svg::Point rhs) {
  return std::tie(lhs.x, lhs.y) == std::tie(rhs.x, rhs.y);
}

bool operator==(svg::Rgb lhs, svg::Rgb rhs) {
  return std::tuple<int, int, int>(lhs.red, lhs.green, lhs.blue)
      == std::tuple<int, int, int>(rhs.red, rhs.green, rhs.blue);
}

bool operator==(svg::Rgba lhs, svg::Rgba rhs) {
  return
      std::tuple<int, int, int, double>(lhs.red, lhs.green, lhs.blue, lhs.alpha)
          == std::tuple<int, int, int, double>(rhs.red,
                                               rhs.green,
                                               rhs.blue,
                                               rhs.alpha);
}

bool operator==(const svg::Color &lhs, const svg::Color &rhs) {
  if (std::holds_alternative<std::string>(lhs) &&
      std::holds_alternative<std::string>(rhs)) {
    return std::get<std::string>(lhs) == std::get<std::string>(rhs);
  }
  if (std::holds_alternative<svg::Rgb>(lhs) &&
      std::holds_alternative<svg::Rgb>(rhs)) {
    return std::get<svg::Rgb>(lhs) == std::get<svg::Rgb>(rhs);
  }
  if (std::holds_alternative<svg::Rgba>(lhs) &&
      std::holds_alternative<svg::Rgba>(rhs)) {
    return std::get<svg::Rgba>(lhs) == std::get<svg::Rgba>(rhs);
  }
  return std::holds_alternative<std::monostate>(lhs) &&
      std::holds_alternative<std::monostate>(rhs);
}

bool operator==(const rm::RenderingSettings &lhs,
                const rm::RenderingSettings &rhs) {
  if (std::tuple(lhs.width,
                 lhs.height,
                 lhs.padding,
                 lhs.stop_radius,
                 lhs.line_width,
                 lhs.underlayer_width,
                 lhs.stop_label_font_size,
                 lhs.color_palette.size()) !=
      std::tuple(rhs.width,
                 rhs.height,
                 rhs.padding,
                 rhs.stop_radius,
                 rhs.line_width,
                 rhs.underlayer_width,
                 rhs.stop_label_font_size,
                 rhs.color_palette.size()))
    return false;
  if (lhs.stop_label_offset != rhs.stop_label_offset) return false;
  if (lhs.underlayer_color != rhs.underlayer_color) return false;
  for (int i = 0; i < lhs.color_palette.size(); ++i) {
    if (lhs.color_palette[i] != rhs.color_palette[i]) return false;
  }
  return true;
}

bool operator==(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs) {
  return std::tie(lhs.stop, lhs.time) == std::tie(rhs.stop, rhs.time);
}

bool operator==(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs) {
  if (!CompareLength(lhs.time, rhs.time, 9)) return false;
  return std::tie(lhs.bus, lhs.span_count)
      == std::tie(rhs.bus, rhs.span_count);
}

bool operator==(const rm::RouteResponse &lhs, const rm::RouteResponse &rhs) {
  return std::tie(lhs.items, lhs.time) == std::tie(rhs.items, rhs.time);
}

bool operator!=(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetRouteRequest &lhs,
                const rm::GetRouteRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetMapRequest &lhs, const rm::GetMapRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::BusResponse &lhs, const rm::BusResponse &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::PostBusRequest &lhs,
                const rm::PostBusRequest &rhs) { return !(lhs == rhs); }

bool operator!=(const rm::PostStopRequest &lhs,
                const rm::PostStopRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::RoutingSettings &lhs,
                const rm::RoutingSettings &rhs) {
  return !(lhs == rhs);
}

bool operator!=(svg::Point lhs, svg::Point rhs) {
  return !(lhs == rhs);
}

bool operator!=(svg::Rgb lhs, svg::Rgb rhs) {
  return !(lhs == rhs);
}

bool operator!=(svg::Rgba lhs, svg::Rgba rhs) {
  return !(lhs == rhs);
}

bool operator!=(const svg::Color &lhs, const svg::Color &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::RenderingSettings lhs,
                const rm::RenderingSettings rhs) {
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

bool operator!=(const rm::RouteResponse &lhs, const rm::RouteResponse &rhs) {
  return !(lhs == rhs);
}

template<typename T>
std::ostream &operator<<(std::ostream &out,
                         const std::vector<T> &str_v) {
  for (auto &item : str_v)
    out << ' ' << item;
  return out;
}

std::ostream &operator<<(std::ostream &out, const rm::PostBusRequest &br) {
  return out << br.bus << ": " << br.stops;
}

std::ostream &operator<<(std::ostream &out, const rm::PostStopRequest &sr) {
  out << sr.stop << ": " << sr.coords.latitude << ' ' <<
      sr.coords.longitude << '\t';
  for (auto [stop, dist] : sr.stop_distances) {
    out << '{' << stop << ':' << dist << "} ";
  }
  return out;
}

std::ostream &operator<<(std::ostream &out, const rm::GetBusRequest &br) {
  return out << br.bus << " – " << br.id;
}

std::ostream &operator<<(std::ostream &out, const rm::GetStopRequest &br) {
  return out << br.stop << " – " << br.id;
}

std::ostream &operator<<(std::ostream &out, const rm::GetMapRequest &mr) {
  return out << mr.id;
}

std::ostream &operator<<(std::ostream &out, const rm::BusResponse &br) {
  return out << br.stop_count << ' ' << br.unique_stop_count << ' '
             << br.length;
}

std::ostream &operator<<(std::ostream &out,
                         const rm::RoutingSettings &settings) {
  return out << settings.bus_wait_time << ' ' << settings.bus_velocity;
}

std::ostream &operator<<(std::ostream &out, const svg::Point point) {
  return out << '{' << point.x << ',' << point.y << '}';
}

std::ostream &operator<<(std::ostream &out,
                         const rm::RenderingSettings &settings) {
  return out << settings.width << ' ' << settings.height << ' ' <<
             settings.padding << ' ' << settings.stop_radius << ' ' <<
             settings.line_width << ' ' << settings.stop_label_font_size
             << ' ' << settings.stop_label_offset << ' ' <<
             settings.underlayer_color << ' ' << settings.underlayer_width
             << ' ' << settings.color_palette;
}

std::ostream &operator<<(std::ostream &out,
                         const rm::RouteResponse::RoadItem &ri) {
  return out << ri.bus << '-' << ri.time << '-' << ri.span_count;
}

std::ostream &operator<<(std::ostream &out, const RouteResponse::WaitItem &wi) {
  return out << wi.stop << '-' << wi.time;
}

std::ostream &operator<<(std::ostream &out, const rm::RouteResponse &rr) {
  return out << rr.time << ' ' << rr.items;
}

std::ostream &operator<<(std::ostream &out,
                         const rm::RouteResponse::Item &item) {
  std::visit([&](auto &&var) {
    out << var;
  }, item);
  return out;
}
}
