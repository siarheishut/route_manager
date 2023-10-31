#include "src/request_processor.h"

#include <optional>
#include <string>
#include <vector>

#include "gtest/gtest.h"

TEST(TestProcessRequests, TestStopResponseToJson) {
  using ResponseOpt = std::optional<rm::StopResponse>;

  struct TestCase {
    std::string name;
    ResponseOpt response;
    int id;
    json::Dict want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Unknown stop",
          .response = std::nullopt,
          .id = 94387,
          .want = json::Dict{{"request_id", 94387},
                             {"error_message", "not found"}},
      },
      TestCase{
          .name = "No buses",
          .response = {{}},
          .id = 5612022,
          .want = json::Dict{{"request_id", 5612022},
                             {"buses", json::List{}}},
      },
      TestCase{
          .name = "Buses found",
          .response = rm::StopResponse{{"Bus1", "12464", "Bus 2"}},
          .id = 12,
          .want = json::Dict{{"request_id", 12},
                             {"buses", json::List{"Bus1", "12464", "Bus 2"}}},
      },
  };

  for (auto &[name, response, id, want] : test_cases) {
    json::Dict got = rm::ToJson(response, id);
    EXPECT_EQ(want, got);
  }
}

TEST(TestProcessRequests, TestBusResponseToJson) {
  using ResponseOpt = std::optional<rm::BusResponse>;

  struct TestCase {
    std::string name;
    ResponseOpt response;
    int id;
    json::Dict want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Unknown bus",
          .response = std::nullopt,
          .id = 1234567,
          .want = json::Dict{{"request_id", 1234567},
                             {"error_message", "not found"}},
      },
      TestCase{
          .name = "Bus Found",
          .response = rm::BusResponse{
              .stop_count = 5,
              .unique_stop_count = 4,
              .length = 194271.1,
              .curvature = 1.54712,
          },
          .id = 1407207,
          .want = json::Dict{{"route_length", 194271.1},
                             {"request_id", 1407207},
                             {"curvature", 1.54712},
                             {"stop_count", 5},
                             {"unique_stop_count", 4}},
      },
  };

  for (auto &[name, response, id, want] : test_cases) {
    json::Dict got = rm::ToJson(response, id);
    EXPECT_EQ(want, got);
  }
}

TEST(TestProcessRequests, TestRouteResponseToJson) {
  using ResponseOpt = std::optional<rm::RouteResponse>;

  struct TestCase {
    std::string name;
    ResponseOpt response;
    std::string map;
    int id;
    json::Dict want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Missing route",
          .response = std::nullopt,
          .map = "some map",
          .id = 1234567,
          .want = json::Dict{{"request_id", 1234567},
                             {"error_message", "not found"}},
      },
      TestCase{
          .name = "Route Found",
          .response = rm::RouteResponse{
              .time = 69.543,
              .items = {
                  rm::RouteResponse::WaitItem{.stop = "stop 1", .time = 5},
                  rm::RouteResponse::RoadItem{
                      .bus = "bus 1", .time = 10.43, .span_count = 4},
                  rm::RouteResponse::WaitItem{.stop = "stop 2", .time = 5},
                  rm::RouteResponse::RoadItem{
                      .bus = "bus 2", .time = 49.113, .span_count = 7}}
          },
          .map = "some map",
          .id = 7421097,
          .want = json::Dict{
              {"total_time", 69.543},
              {"items", json::List{
                  json::Dict{
                      {"type", "Wait"},
                      {"stop_name", "stop 1"},
                      {"time", 5}
                  },
                  json::Dict{
                      {"type", "Bus"},
                      {"bus", "bus 1"},
                      {"time", 10.43},
                      {"span_count", 4}
                  },
                  json::Dict{
                      {"type", "Wait"},
                      {"stop_name", "stop 2"},
                      {"time", 5}
                  },
                  json::Dict{
                      {"type", "Bus"},
                      {"bus", "bus 2"},
                      {"time", 49.113},
                      {"span_count", 7}
                  },
              }},
              {"map", "some map"},
              {"request_id", 7421097}
          }
      },
  };

  for (auto &[name, response, map, id, want] : test_cases) {
    json::Dict got = rm::ToJson(response, map, id);
    EXPECT_EQ(want, got);
  }
}

TEST(TestProcessRequests, TestMapResponseToJson) {
  auto want = json::Dict{{"map", "My map"}, {"request_id", 12}};
  json::Dict got = rm::ToJson(rm::MapResponse{.map = "My map"}, 12);
  EXPECT_EQ(want, got);
}
