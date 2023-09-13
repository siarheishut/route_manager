#ifndef ROOT_MANAGER_TESTS_TEST_UTILS_H_
#define ROOT_MANAGER_TESTS_TEST_UTILS_H_

#include <ostream>
#include <string>
#include <vector>

#include "svg/common.h"

#include "src/bus_manager.h"
#include "src/request_types.h"
#include "svg/common.h"

namespace rm {
bool CompareLength(double lhs, double rhs, int precision);

bool operator==(const rm::BusResponse &lhs, const rm::BusResponse &rhs);

bool operator!=(const rm::BusResponse &lhs, const rm::BusResponse &rhs);

bool operator==(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs);

bool operator!=(const rm::GetBusRequest &lhs, const rm::GetBusRequest &rhs);

bool operator==(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs);

bool operator!=(const rm::GetStopRequest &lhs, const rm::GetStopRequest &rhs);

bool operator==(const rm::GetRouteRequest &lhs, const rm::GetRouteRequest &rhs);

bool operator!=(const rm::GetRouteRequest &lhs, const rm::GetRouteRequest &rhs);

bool operator==(const rm::GetMapRequest &lhs, const rm::GetMapRequest &rhs);

bool operator!=(const rm::GetMapRequest &lhs, const rm::GetMapRequest &rhs);

bool operator==(const rm::PostBusRequest &lhs, const rm::PostBusRequest &rhs);

bool operator!=(const rm::PostBusRequest &lhs, const rm::PostBusRequest &rhs);

bool operator==(const rm::PostStopRequest &lhs, const rm::PostStopRequest &rhs);

bool operator!=(const rm::PostStopRequest &lhs, const rm::PostStopRequest &rhs);

bool operator==(const rm::RoutingSettings lhs, const rm::RoutingSettings rhs);

bool operator!=(const rm::RoutingSettings lhs, const rm::RoutingSettings rhs);

bool operator==(svg::Point lhs, svg::Point rhs);

bool operator!=(svg::Point lhs, svg::Point rhs);

bool operator==(svg::Rgb lhs, svg::Rgb rhs);

bool operator!=(svg::Rgb lhs, svg::Rgb rhs);

bool operator==(svg::Rgba lhs, svg::Rgba rhs);

bool operator!=(svg::Rgba lhs, svg::Rgba rhs);

bool operator==(const svg::Color &lhs, const svg::Color &rhs);

bool operator!=(const svg::Color &lhs, const svg::Color &rhs);

bool operator==(const rm::RenderingSettings &lhs,
                const rm::RenderingSettings &rhs);

bool operator!=(const rm::RenderingSettings &lhs,
                const rm::RenderingSettings &rhs);

bool operator==(svg::Rgb lhs, svg::Rgb rhs);

bool operator!=(svg::Rgb lhs, svg::Rgb rhs);

bool operator==(svg::Rgba lhs, svg::Rgba rhs);

bool operator!=(svg::Rgba lhs, svg::Rgba rhs);

bool operator==(const svg::Color &lhs, const svg::Color &rhs);

bool operator!=(const svg::Color &lhs, const svg::Color &rhs);

bool operator==(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs);

bool operator!=(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs);

bool operator==(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs);

bool operator!=(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs);

bool operator==(const rm::RouteResponse &lhs, const rm::RouteResponse &rhs);

bool operator!=(const rm::RouteResponse &lhs, const rm::RouteResponse &rhs);

std::ostream &operator<<(std::ostream &out, const rm::PostBusRequest &br);

std::ostream &operator<<(std::ostream &out, const rm::PostStopRequest &sr);

std::ostream &operator<<(std::ostream &out, const rm::GetBusRequest &br);

std::ostream &operator<<(std::ostream &out, const rm::GetMapRequest &br);

std::ostream &operator<<(std::ostream &out, const rm::BusResponse &br);

std::ostream &operator<<(std::ostream &out, const rm::RoutingSettings settings);

std::ostream &operator<<(std::ostream &out, svg::Point point);

std::ostream &operator<<(std::ostream &out,
                         const rm::RenderingSettings &settings);

std::ostream &operator<<(std::ostream &out,
                         const rm::RouteResponse::RoadItem &ri);

std::ostream &operator<<(std::ostream &out,
                         const rm::RouteResponse::WaitItem &ri);

std::ostream &operator<<(std::ostream &out, const rm::RouteResponse &rr);

std::ostream &operator<<(std::ostream &out, const RouteResponse::Item &item);
}

#endif // ROOT_MANAGER_TESTS_TEST_UTILS_H_
