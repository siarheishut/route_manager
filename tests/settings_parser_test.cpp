#include <optional>
#include <string>
#include <vector>

#include "json.h"
#include "gtest/gtest.h"

#include "src/request_parser.h"
#include "src/request_types.h"
#include "tests/test_utils.h"

TEST(TestParseSettings, TestRoutingSettings) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<RoutingSettings> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <bus_wait_time>",
          .input = json::Dict{{"bus_velocity", 36.4},
                              {"bus_wait", 10}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: no <bus_velocity>",
          .input = json::Dict{{"bus_speed", 36.4},
                              {"bus_wait_time", 10}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: <bus_wait_time> isn't int",
          .input = json::Dict{{"bus_velocity", 42.2},
                              {"bus_wait_time", 4.7}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: <bus_velocity> isn't double",
          .input = json::Dict{{"bus_velocity", "41.3"},
                              {"bus_wait_time", 6}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Common settings",
          .input = json::Dict{{"bus_velocity", 10.1},
                              {"bus_wait_time", 10}},
          .want = RoutingSettings{.bus_wait_time = 10, .bus_velocity = 10.1}
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseRoutingSettings(input);
    EXPECT_EQ(want, got);
  }
}
