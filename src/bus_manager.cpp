#include "bus_manager.h"

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "svg/common.h"
#include "svg/document.h"
#include "svg/figures.h"

#include "distance_computer.h"

namespace {
int ComputeUniqueCount(std::vector<std::string> stops) {
  std::sort(begin(stops), end(stops));
  return std::distance(stops.begin(),
                       std::unique(begin(stops), end(stops)));
}
}

static constexpr int kMin = std::numeric_limits<int>::min();
static constexpr int kMax = std::numeric_limits<int>::max();

namespace rm {
std::unique_ptr<BusManager> BusManager::Create(
    std::vector<PostRequest> requests,
    RoutingSettings routing_settings,
    RenderingSettings rendering_settings) {
  double min_lon = kMax, max_lon = kMin, min_lat = kMax, max_lat = kMin;
  std::set<std::string_view> route_stops, road_dists_stops, stop_requests_stops;
  std::set<std::string_view> buses;
  for (auto &req : requests) {
    if (auto ptr_b = std::get_if<PostBusRequest>(&req)) {
      route_stops.insert(ptr_b->stops.begin(), ptr_b->stops.end());
      if (!buses.insert(ptr_b->bus).second)
        return nullptr;
    } else if (auto ptr_s = std::get_if<PostStopRequest>(&req)) {
      auto lat = ptr_s->coords.latitude;
      auto lon = ptr_s->coords.longitude;
      max_lat = std::max(max_lat, lat);
      max_lon = std::max(max_lon, lon);
      min_lat = std::min(min_lat, lat);
      min_lon = std::min(min_lon, lon);

      for (auto &[dst_stop, dist] : ptr_s->stop_distances) {
        road_dists_stops.insert(dst_stop);
      }
      if (!stop_requests_stops.insert(ptr_s->stop).second)
        return nullptr;
    }
  }
  if (!std::includes(begin(stop_requests_stops), end(stop_requests_stops),
                     begin(route_stops), end(route_stops)))
    return nullptr;
  if (!std::includes(begin(stop_requests_stops), end(stop_requests_stops),
                     begin(road_dists_stops), end(road_dists_stops)))
    return nullptr;

  CoordsConverter::Config list{.min_lon = min_lon, .max_lon = max_lon,
      .min_lat = min_lat, .max_lat = max_lat,
      .width = rendering_settings.width,
      .height = rendering_settings.height,
      .padding = rendering_settings.padding
  };
  return std::unique_ptr<BusManager>(new BusManager(std::move(requests),
                                                    std::move(routing_settings),
                                                    std::move(rendering_settings),
                                                    list));
}

BusManager::BusManager(std::vector<PostRequest> requests,
                       RoutingSettings routing_settings,
                       RenderingSettings rendering_settings,
                       CoordsConverter::Config config)
    : converter_(CoordsConverter(std::move(config))) {
  std::map<std::string_view, std::vector<std::string_view>> routes;
  std::map<std::string_view, svg::Point> stops;
  for (auto &request : requests) {
    if (std::holds_alternative<PostBusRequest>(request)) {
      auto &bus = std::get<PostBusRequest>(request);
      AddBus(bus.bus, bus.stops);
      routes[bus.bus];
      for (auto &stop : bus.stops) {
        routes[bus.bus].push_back(stop);
      }
    } else if (std::holds_alternative<PostStopRequest>(request)) {
      auto &stop = std::get<PostStopRequest>(request);
      AddStop(stop.stop,
              stop.coords,
              stop.stop_distances);
      stops[stop.stop] = converter_.Convert(stop.coords);
    }
  }

  for (auto &[bus, bus_info] : bus_info_) {
    double geo_dist = ComputeGeoDistance(bus_info.stops, stop_info_);
    bus_info.distance = ComputeRoadDistance(bus_info.stops, stop_info_);
    bus_info.unique_stop_count = ComputeUniqueCount(bus_info.stops);
    bus_info.curvature = bus_info.distance / geo_dist;
  }
  for (auto &[_, info] : stop_info_) {
    auto &buses = info.buses;
    std::sort(buses.begin(), buses.end());
    buses.erase(std::unique(buses.begin(), buses.end()),
                buses.end());
  }
  route_manager_ =
      std::make_unique<RouteManager>(stop_info_,
                                     bus_info_,
                                     std::move(routing_settings));

  RenderMap(std::move(routes), std::move(stops), std::move(rendering_settings));
}

void BusManager::AddStop(const std::string &stop, sphere::Coords coords,
                         const std::map<std::string, int> &stops) {
  constexpr double k = 3.1415926535 / 180;
  auto &info = stop_info_[stop];
  info.coords = {coords.latitude * k, coords.longitude * k};
  for (auto &[stop_to, dist] : stops) {
    auto &stop_to_dists = stop_info_[stop_to].dists;
    if (auto it = stop_to_dists.find(stop); it == stop_to_dists.end()) {
      stop_to_dists[stop] = dist;
    }
    info.dists[stop_to] = dist;
  }
}

void BusManager::AddBus(std::string bus,
                        std::vector<std::string> stops) {
  for (auto &stop : stops)
    stop_info_[stop].buses.push_back(bus);

  BusInfo bus_info;
  bus_info.stops = std::move(stops);
  bus_info_[std::move(bus)] = bus_info;
}

std::optional<BusResponse> BusManager::GetBusInfo(const std::string &bus) const {
  auto it = bus_info_.find(bus);
  if (it == bus_info_.end()) return std::nullopt;

  auto &b = it->second;
  return BusResponse{
      .stop_count = static_cast<int>(b.stops.size()),
      .unique_stop_count = b.unique_stop_count,
      .length = b.distance,
      .curvature = b.curvature,
  };
}

std::optional<StopResponse> BusManager::GetStopInfo(const std::string &stop) const {
  auto it = stop_info_.find(stop);
  if (it == stop_info_.end())
    return std::nullopt;

  return StopResponse{it->second.buses};
}

std::optional<RouteResponse> BusManager::GetRoute(const std::string &from,
                                                  const std::string &to) const {
  return route_manager_->FindRoute(from, to);
}

Map BusManager::GetMap() const {
  return map_;
}

void BusManager::RenderMap(std::map<std::string_view,
                                    std::vector<std::string_view>> routes,
                           std::map<std::string_view, svg::Point> stops,
                           RenderingSettings settings) {
  svg::Document doc;
  int counter = 0;
  for (auto &[_, route_points] : routes) {
    svg::Polyline bus_route{};
    bus_route.SetStrokeColor(
        settings.color_palette[counter++ % settings.color_palette.size()]);
    bus_route.SetStrokeLineCap("round");
    bus_route.SetStrokeLineJoin("round");
    bus_route.SetStrokeWidth(settings.line_width);
    for (auto point : route_points) {
      bus_route.AddPoint(stops[point]);
    }
    doc.Add(std::move(bus_route));
  }

  for (auto [_, stop_point] : stops) {
    svg::Circle stop{};
    stop.SetFillColor("white");
    stop.SetCenter(stop_point);
    stop.SetRadius(settings.stop_radius);
    doc.Add(std::move(stop));
  }

  for (auto [stop_name, stop_point] : stops) {
    svg::Text underlayer;
    underlayer.SetPoint(stop_point);
    underlayer.SetOffset(settings.stop_label_offset);
    underlayer.SetFontSize(settings.stop_label_font_size);
    underlayer.SetFontFamily("Verdana");
    underlayer.SetData(std::string(stop_name));
    underlayer.SetFillColor(settings.underlayer_color);
    underlayer.SetStrokeColor(settings.underlayer_color);
    underlayer.SetStrokeWidth(settings.underlayer_width);
    underlayer.SetStrokeLineCap("round");
    underlayer.SetStrokeLineJoin("round");
    doc.Add(std::move(underlayer));
    svg::Text text;
    text.SetPoint(stop_point);
    text.SetOffset(settings.stop_label_offset);
    text.SetFontSize(settings.stop_label_font_size);
    text.SetFontFamily("Verdana");
    text.SetData(std::string(stop_name));
    text.SetFillColor("black");
    doc.Add(std::move(text));
  }

  std::ostringstream ss;
  doc.Render(ss);
  map_ = ss.str();
}
}
