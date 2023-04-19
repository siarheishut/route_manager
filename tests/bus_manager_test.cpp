#include <gtest/gtest.h>

#include "test_utils.h"

#include <optional>
#include <string>
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
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"}},
              PostBusRequest{
                  .bus = "Bus2",
                  .stops = {"stop4", "stop5", "stop6", "stop7", "stop8",
                            "stop4"}},
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {55.611087, 37.20829},
                  .stops = {{"stop2", 3000}}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.595884, 37.209755}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {55.632761, 37.333324}},
              PostStopRequest{ // 3000 + 4000 + 4000 +
                  .stop = "stop4",
                  .coords = {55.574371, 37.6517},
                  .stops = {{"stop5", 4000}, {"stop3", 4000}}},
              PostStopRequest{
                  .stop = "stop5",
                  .coords = {55.581065, 37.64839}},
              PostStopRequest{
                  .stop = "stop6",
                  .coords = {55.587655, 37.645687}},
              PostStopRequest{
                  .stop = "stop7",
                  .coords = {55.592028, 37.653656}},
              PostStopRequest{
                  .stop = "stop8",
                  .coords = {55.580999, 37.659164}},
          },
          .requests = {GetBusRequest{.bus = "Bus1"},
                       GetBusRequest{.bus = "Bus2"},
                       GetBusRequest{.bus = "none"}},
          .want = {BusResponse{5, 3, 2.35535e+04},
                   BusResponse{6, 5, 7598.15},
                   std::nullopt},
      },
  };

  for (auto &[name, test_item, requests, want] : test_cases) {
    BusManager bm(test_item);

    for (int i = 0; i < requests.size(); ++i) {
      auto got = bm.GetBusInfo(requests[i].bus);

      if (!want[i].has_value()) {
        EXPECT_TRUE(!got.has_value()) << name;
        continue;
      }

      std::pair p1{want[i]->stop_count, want[i]->unique_stop_count};
      std::pair p2{got->stop_count, got->unique_stop_count};
      EXPECT_EQ(p1, p2) << name;

      if (want[i] != std::nullopt)
        EXPECT_TRUE(CompareLength(want[i]->length, got->length, 6))
                  << name << " – " << std::setprecision(6) << "want: "
                  << want[i]->length << ", got: " << got->length;
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
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"}},
              PostBusRequest{
                  .bus = "Bus2",
                  .stops = {"stop3", "stop4", "stop5", "stop6", "stop7",
                            "stop3"}},
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {55.611087, 37.20829}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.595884, 37.209755}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {55.632761, 37.333324}},
              PostStopRequest{
                  .stop = "stop4",
                  .coords = {55.574371, 37.6517}},
              PostStopRequest{
                  .stop = "stop5",
                  .coords = {55.581065, 37.64839}},
              PostStopRequest{
                  .stop = "stop6",
                  .coords = {55.587655, 37.645687}},
              PostStopRequest{
                  .stop = "stop7",
                  .coords = {55.592028, 37.653656}},
              PostStopRequest{
                  .stop = "stop8",
                  .coords = {55.580999, 37.659164}},
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
        EXPECT_TRUE(!got.has_value()) << name;
        continue;
      }

      EXPECT_EQ(want[i]->buses, got->buses) << name;
    }
  }
}
