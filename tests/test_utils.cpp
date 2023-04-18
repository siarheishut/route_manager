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

bool operator==(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs) {
  return lhs.bus == rhs.bus;
}

bool operator==(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs) {
  return lhs.stop == rhs.stop;
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
  return std::tie(lhs.stop, lhs.coords.latitude, lhs.coords.longitude) ==
      std::tie(rhs.stop, rhs.coords.latitude, rhs.coords.longitude);
}

bool operator!=(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs) {
  return !(lhs == rhs);
}

bool operator!=(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs) {
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

std::ostream &operator<<(std::ostream &out,
                         const std::vector<std::string> &str_v) {
  for (auto &str : str_v)
    out << str << ' ';
  return out;
}

std::ostream &operator<<(std::ostream &out, const rm::PostBusRequest &br) {
  return out << br.bus << ": " << br.stops;
}

std::ostream &operator<<(std::ostream &out, const rm::PostStopRequest &sr) {
  return out << sr.stop << ": " << std::setprecision(6) << sr.coords.latitude
             << ' ' << std::setprecision(6) << sr.coords.longitude;
}

std::ostream &operator<<(std::ostream &out, const rm::GetBusRequest &br) {
  return out << br.bus;
}

std::ostream &operator<<(std::ostream &out, const rm::GetStopRequest &br) {
  return out << br.stop;
}

std::ostream &operator<<(std::ostream &out, const rm::BusResponse &br) {
  return out << br.stop_count << ' ' << br.unique_stop_count << ' '
             << br.length;
}
}
