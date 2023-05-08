#ifndef ROOT_MANAGER_TESTS_TEST_UTILS_H_
#define ROOT_MANAGER_TESTS_TEST_UTILS_H_

#include "src/bus_manager.h"
#include "src/request_types.h"

#include <ostream>
#include <string>
#include <vector>

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

bool operator==(const rm::PostBusRequest &lhs, const rm::PostBusRequest &rhs);

bool operator!=(const rm::PostBusRequest &lhs, const rm::PostBusRequest &rhs);

bool operator==(const rm::PostStopRequest &lhs, const rm::PostStopRequest &rhs);

bool operator!=(const rm::PostStopRequest &lhs, const rm::PostStopRequest &rhs);

std::ostream &operator<<(std::ostream &out, const rm::PostBusRequest &br);

std::ostream &operator<<(std::ostream &out, const rm::PostStopRequest &sr);

std::ostream &operator<<(std::ostream &out, const rm::GetBusRequest &br);

std::ostream &operator<<(std::ostream &out, const rm::BusResponse &br);
}

#endif // ROOT_MANAGER_TESTS_TEST_UTILS_H_
