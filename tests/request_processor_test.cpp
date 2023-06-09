#include <gtest/gtest.h>

#include "src/request_processor.h"

#include <string>
#include <vector>
#include <optional>

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
  using json::List;
  using json::Dict;
  using ResponseOpt = std::optional<rm::RouteResponse>;

  struct TestCase {
    std::string name;
    ResponseOpt response;
    int id;
    Dict want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "No route",
          .response = std::nullopt,
          .id = 1234567,
          .want = Dict{{"request_id", 1234567},
                       {"error_message", "not found"}},
      },
      TestCase{
          .name = "Bus Found",
          .response = rm::RouteResponse{
              .time = 25.135,
              .items = {
                  rm::RouteResponse::WaitItem{
                      .stop = "Stop 1",
                      .time = 12},
                  rm::RouteResponse::RoadItem{
                      .bus = "Bus 1",
                      .time = 13.135,
                      .span_count = 3},
              },
          },
          .id = 1407207,
          .want = Dict{{"total_time", 25.135},
                       {"items", List{Dict{{"time", 12}, {"type", "Wait"},
                                           {"stop_name", "Stop 1"}},
                                      Dict{{"time", 13.135}, {"type", "Bus"},
                                           {"bus", "Bus 1"}, {"span_count", 3}}
                       }},
                       {"request_id", 1407207}},
      },
  };

  for (auto &[name, response, id, want] : test_cases) {
    Dict got = rm::ToJson(response, id);
    EXPECT_EQ(want, got);
  }
}
