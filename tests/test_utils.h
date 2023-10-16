#ifndef ROOT_MANAGER_TESTS_TEST_UTILS_H_
#define ROOT_MANAGER_TESTS_TEST_UTILS_H_

#include <ostream>
#include <string>
#include <vector>

#include "svg/common.h"

#include "src/bus_manager.h"
#include "svg/common.h"
#include "src/map_renderer_utils.h"
#include "src/request_types.h"

namespace rm {
bool CompareLength(double lhs, double rhs, int precision);

bool operator==(const BusResponse &lhs, const BusResponse &rhs);

bool operator!=(const BusResponse &lhs, const BusResponse &rhs);

bool operator==(const GetBusRequest &lhs, const GetBusRequest &rhs);

bool operator!=(const GetBusRequest &lhs, const GetBusRequest &rhs);

bool operator==(const GetStopRequest &lhs, const GetStopRequest &rhs);

bool operator!=(const GetStopRequest &lhs, const GetStopRequest &rhs);

bool operator==(const GetRouteRequest &lhs, const GetRouteRequest &rhs);

bool operator!=(const GetRouteRequest &lhs, const GetRouteRequest &rhs);

bool operator==(const GetMapRequest &lhs, const GetMapRequest &rhs);

bool operator!=(const GetMapRequest &lhs, const GetMapRequest &rhs);

bool operator==(const PostBusRequest &lhs, const PostBusRequest &rhs);

bool operator!=(const PostBusRequest &lhs, const PostBusRequest &rhs);

bool operator==(const PostStopRequest &lhs, const PostStopRequest &rhs);

bool operator!=(const PostStopRequest &lhs, const PostStopRequest &rhs);

bool operator==(const RoutingSettings &lhs, const RoutingSettings &rhs);

bool operator!=(const RoutingSettings &lhs, const RoutingSettings &rhs);

bool operator==(const RenderingSettings &lhs, const RenderingSettings &rhs);

bool operator!=(const RenderingSettings &lhs, const RenderingSettings &rhs);

bool operator==(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs);

bool operator!=(const RouteResponse::WaitItem &lhs,
                const RouteResponse::WaitItem &rhs);

bool operator==(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs);

bool operator!=(const RouteResponse::RoadItem &lhs,
                const RouteResponse::RoadItem &rhs);

bool operator==(const RouteResponse &lhs, const RouteResponse &rhs);

bool operator!=(const RouteResponse &lhs, const RouteResponse &rhs);

std::ostream &operator<<(std::ostream &out, const PostBusRequest &br);

std::ostream &operator<<(std::ostream &out, const PostStopRequest &sr);

std::ostream &operator<<(std::ostream &out, const GetBusRequest &br);

std::ostream &operator<<(std::ostream &out, const GetMapRequest &br);

std::ostream &operator<<(std::ostream &out, const BusResponse &br);

std::ostream &operator<<(std::ostream &out, const RoutingSettings &settings);

std::ostream &operator<<(std::ostream &out, const RenderingSettings &settings);

std::ostream &operator<<(std::ostream &out, const RouteResponse::RoadItem &ri);

std::ostream &operator<<(std::ostream &out, const RouteResponse::WaitItem &ri);

std::ostream &operator<<(std::ostream &out, const RouteResponse::Item &item);

std::ostream &operator<<(std::ostream &out, const RouteResponse &rr);
}

namespace rm::renderer_utils {
bool operator==(const Frame &lhs, const Frame &rhs);

bool operator!=(const Frame &lhs, const Frame &rhs);

std::ostream &operator<<(std::ostream &out, const Frame &frame);
}

namespace svg {
bool operator==(Point lhs, Point rhs);

bool operator!=(Point lhs, Point rhs);

bool operator==(Rgb lhs, Rgb rhs);

bool operator!=(Rgb lhs, Rgb rhs);

bool operator==(Rgba lhs, Rgba rhs);

bool operator!=(Rgba lhs, Rgba rhs);

bool operator==(const Color &lhs, const Color &rhs);

bool operator!=(const Color &lhs, const Color &rhs);

std::ostream &operator<<(std::ostream &out, Point point);
}

#endif // ROOT_MANAGER_TESTS_TEST_UTILS_H_
