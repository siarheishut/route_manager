#include "src/request_reader.h"
#include "src/utils.h"

#include <charconv>
#include <istream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <exception>

namespace {
std::optional<int> ToInt(std::string_view sv) {
  sv = rm::TrimEnd(rm::TrimStart(sv));
  int ans;
  auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), ans);
  if (ec != std::errc{} || ptr != sv.end()) return std::nullopt;
  return ans;
}

std::optional<double> ToDouble(std::string_view sv) {
  std::stringstream ss;
  ss << sv;
  double result;
  if (ss >> result && ss.eof()) return result;
  return std::nullopt;
}
}

namespace rm {
std::optional<std::vector<PostRequest>> ReadInputRequests(std::istream &in) {
  std::vector<PostRequest> requests;

  std::string line;

  std::getline(in, line);
  auto line_count = ToInt(line);
  if (!line_count) return std::nullopt;

  while ((*line_count)--) {
    std::getline(in, line);
    std::string_view sv = line;

    auto key = ReadNextToken(sv, " ");

    if (key == "Bus") {
      if (auto br = ParsePostBusRequest(sv))
        requests.emplace_back(std::move(*br));
    }
    if (key == "Stop") {
      if (auto sr = ParsePostStopRequest(sv))
        requests.emplace_back(std::move(*sr));
    }
  }
  return requests;
}

std::optional<std::vector<GetRequest>> ReadOutputRequests(std::istream &in) {
  std::vector<GetRequest> requests;

  std::string line;

  std::getline(in, line);
  auto line_count = ToInt(line);
  if (!line_count) return std::nullopt;

  while ((*line_count)--) {
    std::getline(in, line);
    std::string_view sv = line;

    auto key = ReadNextToken(sv, " ");

    if (key == "Bus") {
      if (auto br = ParseGetBusRequest(sv))
        requests.emplace_back(std::move(*br));
    }
    if (key == "Stop") {
      if (auto sr = ParseGetStopRequest(sv))
        requests.emplace_back(std::move(*sr));
    }
  }

  return requests;
}

std::optional<PostBusRequest> ParsePostBusRequest(std::string_view sv) {
  auto pos1 = sv.find_first_of('-');
  auto pos2 = sv.find_first_of('>');
  if (pos1 == sv.npos ^ pos2 != sv.npos)
    return std::nullopt;

  if (auto ch = sv[sv.find_last_not_of(' ')]; ch == '>' || ch == '-')
    return std::nullopt;

  PostBusRequest br;
  br.bus = ReadNextToken(sv, ":");
  if (br.bus.empty()) return std::nullopt;

  bool is_circular = (pos1 == sv.npos);
  while (!sv.empty())
    br.stops.emplace_back(ReadNextToken(sv, "->"));

  if (is_circular && br.stops.front() != br.stops.back()) return std::nullopt;
  if (br.stops.size() < 2) return std::nullopt;

  if (!is_circular)
    for (int i = br.stops.size() - 2; i >= 0; --i)
      br.stops.push_back(br.stops[i]);

  return br;
}

std::optional<PostStopRequest> ParsePostStopRequest(std::string_view sv) {
  PostStopRequest sr;
  sr.stop = ReadNextToken(sv, ":");
  if (sr.stop.empty()) return std::nullopt;

  auto pos = sv.find_first_of(',');
  if (pos == sv.npos) return std::nullopt;

  auto lat = ToDouble(ReadNextToken(sv, ","));
  auto lon = ToDouble(ReadNextToken(sv, ""));
  if (!lat || !lon) return std::nullopt;

  try {
    sr.coords.latitude = *lat;
    sr.coords.longitude = *lon;
  } catch (std::exception &ex) {
    return std::nullopt;
  }
  return sr;
}

std::optional<GetBusRequest> ParseGetBusRequest(std::string_view sv) {
  std::string_view bus = ReadNextToken(sv, "");
  if (bus.empty()) return std::nullopt;
  return GetBusRequest{.bus = std::string(bus)};
}

std::optional<GetStopRequest> ParseGetStopRequest(std::string_view sv) {
  std::string_view stop = ReadNextToken(sv, "");
  if (stop.empty()) return std::nullopt;
  return GetStopRequest{.stop = std::string(stop)};
}
}
