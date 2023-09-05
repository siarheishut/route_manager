#include "test_utils.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace rm {
bool CompareLength(double lhs, double rhs, int precision) {
  std::stringstream ssl, ssr;
  ssl << std::setprecision(precision) << lhs;
  ssr << std::setprecision(precision) << rhs;
  return ssl.str() == ssr.str();
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

bool operator==(const rm::BusResponse &lhs, const rm::BusResponse &rhs) {
  return std::tie(lhs.stop_count, lhs.unique_stop_count, lhs.length)
      == std::tie(rhs.stop_count, rhs.unique_stop_count, rhs.length);
}

bool operator==(const rm::RouteResponse &lhs, const rm::RouteResponse &rhs) {
  return std::tie(lhs.items, lhs.time) == std::tie(rhs.items, rhs.time);
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

bool operator!=(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs) {
  return !(lhs == rhs);
}
bool operator!=(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetRouteRequest &lhs,
                const rm::GetRouteRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::BusResponse &lhs, const rm::BusResponse &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::RouteResponse &lhs, const rm::RouteResponse &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::PostBusRequest &lhs,
                const rm::PostBusRequest &rhs) { return !(lhs == rhs); }

bool operator!=(const rm::PostStopRequest &lhs,
                const rm::PostStopRequest &rhs) {
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

std::ostream &operator<<(std::ostream &out, const rm::BusResponse &br) {
  return out << br.stop_count << ' ' << br.unique_stop_count << ' '
             << br.length;
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

std::ostream &operator<<(std::ostream &out, const RouteResponse::Item &item) {
  std::visit([&](auto &&var) {
    out << var;
  }, item);
  return out;
}

}

