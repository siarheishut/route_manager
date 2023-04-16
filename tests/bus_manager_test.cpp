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
          .name = "requests to an empty database",
          .config = {},
          .requests = {GetBusRequest{.bus = "123"},
                       GetBusRequest{.bus = "some bus"},
                       GetBusRequest{.bus = "s      s"},},
          .want = {std::nullopt, std::nullopt, std::nullopt},
      },
      TestCase{
          .name = "common requests",
          .config = {
              PostBusRequest{"Nums 123",
                             {"It", "is", "test", "is", "It"}},
              PostBusRequest{"X X 4",
                             {"First stop", "second", "third",
                              "forth", "fifth", "First stop"}},
              PostStopRequest{"It", {55.611087, 37.20829}},
              PostStopRequest{"is", {55.595884, 37.209755}},
              PostStopRequest{"test", {55.632761, 37.333324}},
              PostStopRequest{"First stop", {55.574371, 37.6517}},
              PostStopRequest{"second", {55.581065, 37.64839}},
              PostStopRequest{"third", {55.587655, 37.645687}},
              PostStopRequest{"forth", {55.592028, 37.653656}},
              PostStopRequest{"fifth", {55.580999, 37.659164}},
          },
          .requests = {GetBusRequest{.bus = "Nums 123"},
                       GetBusRequest{.bus = "X X 4"},
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
