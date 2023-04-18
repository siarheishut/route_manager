#include <gtest/gtest.h>

#include "test_utils.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

TEST(TestBusManager, TestGetBusInfo) {
  using namespace rm;

  struct TestCase {
    std::string name;
    std::vector<PostRequest> config;
    std::vector<GetBusRequest> requests;
    std::vector<std::optional<BusResponse>> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {},
          .requests = {GetBusRequest{.bus = "123"},
                       GetBusRequest{.bus = "some bus"},
                       GetBusRequest{.bus = "s      s"},},
          .want = {std::nullopt, std::nullopt, std::nullopt},
      },
      TestCase{
          .name = "Common requests",
          .config = {
              PostBusRequest{"Bus1",
                             {"stop1", "stop2", "stop3", "stop2", "stop1"}},
              PostBusRequest{"Bus2",
                             {"stop4", "stop5", "stop6",
                              "stop7", "stop8", "stop4"}},
              PostStopRequest{"stop1", {55.611087, 37.20829}},
              PostStopRequest{"stop2", {55.595884, 37.209755}},
              PostStopRequest{"stop3", {55.632761, 37.333324}},
              PostStopRequest{"stop4", {55.574371, 37.6517}},
              PostStopRequest{"stop5", {55.581065, 37.64839}},
              PostStopRequest{"stop6", {55.587655, 37.645687}},
              PostStopRequest{"stop7", {55.592028, 37.653656}},
              PostStopRequest{"stop8", {55.580999, 37.659164}},
          },
          .requests = {GetBusRequest{.bus = "Bus1"},
                       GetBusRequest{.bus = "Bus2"},
                       GetBusRequest{.bus = "none"}},
          .want = {BusResponse{5, 3, 20939.5},
                   BusResponse{6, 5, 4371.02},
                   std::nullopt},
      },
  };

  for (auto &[name, test_item, requests, want] : test_cases) {
    BusManager bm(test_item);

    for (int i = 0; i < requests.size(); ++i) {
      auto got = bm.GetBusInfo(requests[i].bus);

      if (!want[i].has_value()) {
        EXPECT_TRUE(!got.has_value());
        continue;
      }

      std::pair p1{want[i]->stop_count, want[i]->unique_stop_count};
      std::pair p2{got->stop_count, got->unique_stop_count};
      EXPECT_EQ(p1, p2) << name;

      if (want[i] != std::nullopt)
        EXPECT_TRUE(CompareLength(want[i]->length, got->length, 6))
                  << std::setprecision(6) << "want: " << want[i]->length
                  << ", got: " << got->length;
    }
  }
}

TEST(TestBusManager, TestGetStopInfo) {
  using namespace rm;

  struct TestCase {
    std::string name;
    std::vector<PostRequest> config;
    std::vector<GetStopRequest> requests;
    std::vector<std::optional<StopInfo>> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {},
          .requests = {GetStopRequest{.stop = "123"},
                       GetStopRequest{.stop = "some stop"},
                       GetStopRequest{.stop = "s      s"},},
          .want = {std::nullopt, std::nullopt, std::nullopt},
      },
      TestCase{
          .name = "Common requests",
          .config = {
              PostBusRequest{"Bus1",
                             {"stop1", "stop2", "stop3", "stop2", "stop1"}},
              PostBusRequest{"Bus2",
                             {"stop3", "stop4", "stop5",
                              "stop6", "stop7", "stop3"}},
              PostStopRequest{"stop1", {55.611087, 37.20829}},
              PostStopRequest{"stop2", {55.595884, 37.209755}},
              PostStopRequest{"stop3", {55.632761, 37.333324}},
              PostStopRequest{"stop4", {55.574371, 37.6517}},
              PostStopRequest{"stop5", {55.581065, 37.64839}},
              PostStopRequest{"stop6", {55.587655, 37.645687}},
              PostStopRequest{"stop7", {55.592028, 37.653656}},
              PostStopRequest{"stop8", {55.580999, 37.659164}},
          },
          .requests = {GetStopRequest{.stop = "stop1"},
                       GetStopRequest{.stop = "stop3"},
                       GetStopRequest{.stop = "stop8"},
                       GetStopRequest{.stop = "stop10"}},
          .want = {
              StopInfo{.buses = {"Bus1"}},
              StopInfo{.buses = {"Bus1", "Bus2"}},
              StopInfo{},
              std::nullopt},
      },
  };

  for (auto &[name, test_item, requests, want] : test_cases) {
    BusManager bm(test_item);

    for (int i = 0; i < requests.size(); ++i) {
      auto got = bm.GetStopInfo(requests[i].stop);

      if (!want[i].has_value()) {
        EXPECT_TRUE(!got.has_value());
        continue;
      }

      EXPECT_EQ(want[i]->buses, got->buses) << name;
    }
  }
}
