#include "src/request_parser.h"

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "gtest/gtest.h"
#include "json.h"

#include "test_utils.h"

TEST(TestInputRequest, TestCommonProperties) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    bool want_fail;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Request doesn't contain field <type>",
          .input = json::Dict{
              {"name", "Bus 1"},
              {"stops", json::List{"stop1", "stop2", "stop1"}},
              {"is_roundtrip", true}},
          .want_fail = true
      },
      TestCase{
          .name = "Type isn't string",
          .input = json::Dict{
              {"type", json::List{"Bus"}},
              {"name", "Bus 1"},
              {"stops", json::List{"stop1", "stop2", "stop1"}},
              {"is_roundtrip", true}},
          .want_fail = true
      },
      TestCase{
          .name = "Unknown request type",
          .input = json::Dict{
              {"type", "stop"},
              {"name", "Stop 1"},
              {"longitude", 10},
              {"latitude", 25},
              {"road_distances", json::Dict{}}},
          .want_fail = true
      },
      TestCase{
          .name = "Valid PostBusRequest",
          .input = json::Dict{
              {"type", "Bus"},
              {"name", "Bus 1"},
              {"stops", json::List{"stop1", "stop2", "stop1"}},
              {"is_roundtrip", true}},
          .want_fail = false
      },
      TestCase{
          .name = "Valid PostStopRequest",
          .input = json::Dict{
              {"type", "Stop"},
              {"name", "Stop 1"},
              {"longitude", 10},
              {"latitude", 25},
              {"road_distances", json::Dict{}}},
          .want_fail = false
      },
  };

  for (auto &[name, input, want_fail] : test_cases) {
    auto got = !ParseInputRequest(input).has_value();
    EXPECT_EQ(got, want_fail) << name;
  }
}

TEST(TestOutputRequest, TestCommonProperties) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    bool want_fail;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Request doesn't contain field <type>",
          .input = json::Dict{{"name", "1"}, {"id", 1}},
          .want_fail = true
      },
      TestCase{
          .name = "Type isn't string",
          .input = json::Dict{{"type", json::List{"Map"}}, {"id", 1}},
          .want_fail = true
      },
      TestCase{
          .name = "Unknown request type",
          .input = json::Dict{{"type", "stop"}, {"name", "Stop 1"}, {"id", 1}},
          .want_fail = true
      },
      TestCase{
          .name = "Valid GetBusRequest",
          .input = json::Dict{{"type", "Bus"}, {"name", "1"}, {"id", 1}},
          .want_fail = false
      },
      TestCase{
          .name = "Valid GetStopRequest",
          .input = json::Dict{{"type", "Stop"}, {"name", "1"}, {"id", 1}},
          .want_fail = false
      },
      TestCase{
          .name = "Valid GetRouteRequest",
          .input = json::Dict{
              {"type", "Route"},
              {"from", "1"},
              {"to", "2"},
              {"id", 1}},
          .want_fail = false
      },
      TestCase{
          .name = "Valid GetMapRequest",
          .input = json::Dict{{"type", "Map"}, {"id", 1}},
          .want_fail = false
      },
  };

  for (auto &[name, input, want_fail] : test_cases) {
    auto got = !ParseOutputRequest(input).has_value();
    EXPECT_EQ(got, want_fail) << name;
  }
}

TEST(TestInputRequest, TestPostBusRequest) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<PostBusRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: different start and end stops on circular route>",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"stops",
                               json::List{"stop1", "stop2", "stop3", "stop11"}},
                              {"is_roundtrip", true}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <bus_name>",
          .input = json::Dict{{"type", "Bus"},
                              {"stops",
                               json::List{"stop1", "stop2", "stop3", "stop1"}},
                              {"is_roundtrip", true}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"no <bus_name>\"",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"stops",
                               json::List{"stop1", "stop2", "stop3", "stop1"}},
                              {"is_roundtrip", true}},
          .want = PostBusRequest{
              .bus = "Bus 1",
              .stops = {"stop1", "stop2", "stop3", "stop1"},
              .is_roundtrip = true
          },
      },
      TestCase{
          .name = "Wrong request: no <is_roundtrip>",
          .input = json::Dict{{"type", "Bus"},
                              {"stops",
                               json::List{"stop1", "stop2", "stop3"}},
                              {"name", "Bus 1"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"no <is_roundtrip>\"",
          .input = json::Dict{{"type", "Bus"},
                              {"stops",
                               json::List{"stop1", "stop2", "stop3"}},
                              {"name", "Bus 1"},
                              {"is_roundtrip", false}},
          .want = PostBusRequest{
              .bus = "Bus 1",
              .stops = {"stop1", "stop2", "stop3"},
              .is_roundtrip = false
          },
      },
      TestCase{
          .name = "Wrong request: no <stops>",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"is_roundtrip", false}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"no <stops>\"",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"is_roundtrip", false},
                              {"stops", json::List{"stop1", "stop2", "stop3"}}},
          .want = PostBusRequest{
              .bus = "Bus 1",
              .stops = {"stop1", "stop2", "stop3"},
              .is_roundtrip = false
          },
      },
      TestCase{
          .name = "Wrong request: one stop",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"stops", json::List{"stop1 stop2"}},
                              {"is_roundtrip", false}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed: \"one stop\"",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"stops", json::List{"stop1", "stop2"}},
                              {"is_roundtrip", false}},
          .want = PostBusRequest{
              .bus = "Bus 1",
              .stops = {"stop1", "stop2"},
              .is_roundtrip = false
          },
      },
      TestCase{
          .name = "Roundtrip is false",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"},
                              {"stops",
                               json::List{"stop1", "stop2", "stop3"}},
                              {"is_roundtrip", false}},
          .want = PostBusRequest{
              .bus = "Bus1",
              .stops = {"stop1", "stop2", "stop3"},
              .is_roundtrip = false
          },
      },
      TestCase{
          .name = "Roundtrip is true",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"},
                              {"stops",
                               json::List{"stop 1", "stop 2", "stop 3",
                                          "stop 1"}},
                              {"is_roundtrip", true}},
          .want = PostBusRequest{
              .bus = "Bus1",
              .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              .is_roundtrip = true,
          }
      },
      TestCase{
          .name = "<name> isn't string",
          .input = json::Dict{{"type", "Bus"},
                              {"name", json::List{"Bus1"}},
                              {"stops",
                               json::List{"stop 1", "stop 2", "stop 3",
                                          "stop 1"}},
                              {"is_roundtrip", true}},
          .want = std::nullopt
      },
      TestCase{
          .name = "<stops> isn't array",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"},
                              {"stops", "Stop1"},
                              {"is_roundtrip", true}},
          .want = std::nullopt
      },
      TestCase{
          .name = "<roundtrip> isn't bool",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"},
                              {"stops",
                               json::List{"stop 1", "stop 2", "stop 3",
                                          "stop 1"}},
                              {"is_roundtrip", "true"}},
          .want = std::nullopt
      },
      TestCase{
          .name = "<stops> contains not string",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"},
                              {"stops",
                               json::List{"stop 1", "stop 2", 3,
                                          "stop 1"}},
                              {"is_roundtrip", true}},
          .want = std::nullopt
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParsePostBusRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestInputRequest, TestPostStopRequest) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<PostStopRequest> want;
  };
  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <road_distances>",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"latitude", 38.131420},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <latitude>",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <longitude>",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", 38.131420}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <name>",
          .input = json::Dict{{"type", "Stop"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", 38.131420},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<name> isn't string",
          .input = json::Dict{{"type", "Stop"},
                              {"name", 1},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", 38.131420},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<road_distances> isn't map",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::List{"stop2", "stop3"}},
                              {"latitude", 38.131420},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<latitude> isn't double",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", "3"},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<longitude> isn't double",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", 38.131420},
                              {"longitude", "5"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<road_distances> value isn't int",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900.4}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", 38.131420},
                              {"longitude", -95.538143}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Positive coords",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop1"},
                              {"road_distances",
                               json::Dict{{"stop2", 3900}, {"stop3", 31333},
                                          {"stop4", 11200}}},
                              {"latitude", 38.131420},
                              {"longitude", 95.538143}},
          .want = PostStopRequest{
              .stop = "stop1",
              .coords = {
                  .latitude = 38.131420,
                  .longitude = 95.538143},
              .stop_distances = {{"stop2", 3900}, {"stop3", 31333},
                                 {"stop4", 11200}},
          }
      },
      TestCase{
          .name = "Negative coords",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop   1"},
                              {"road_distances",
                               json::Dict{{"stop 2", 3900}, {"stop   3", 31333},
                                          {"stop 4", 11200}}},
                              {"latitude", -38.131420},
                              {"longitude", -95.538143}},
          .want = PostStopRequest{
              .stop = "stop   1",
              .coords = {
                  .latitude = -38.131420,
                  .longitude = -95.538143},
              .stop_distances = {{"stop 2", 3900}, {"stop   3", 31333},
                                 {"stop 4", 11200}},
          }
      },
      TestCase{
          .name = "Different sign coords + empty road_distances",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "stop   1"},
                              {"road_distances", json::Dict{}},
                              {"latitude", -38.131420},
                              {"longitude", -95.538143}},
          .want = PostStopRequest{
              .stop = "stop   1",
              .coords = {
                  .latitude = -38.131420,
                  .longitude = -95.538143},
              .stop_distances = {},
          }
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParsePostStopRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestOutputRequest, TestGetBusRequest) {
  using namespace rm;
  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<GetBusRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <name>",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1965312327}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <id>",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<id> isn't int",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1.2},
                              {"name", "Bus1"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<name> isn't string",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1},
                              {"name", 1}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Common request",
          .input = json::Dict{{"type", "Bus"},
                              {"name", "Bus1"},
                              {"id", 12412876}},
          .want = GetBusRequest{
              .id = 12412876,
              .bus = "Bus1",}
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseGetBusRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestOutputRequest, TestGetRouteRequest) {
  using namespace rm;
  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<GetRouteRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <from>",
          .input = json::Dict{{"type", "Route"},
                              {"id", 1},
                              {"to", "stop 2"},
          },
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <to>",
          .input = json::Dict{{"type", "Route"},
                              {"id", 2},
                              {"from", "stop 1"},
          },
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <id>",
          .input = json::Dict{{"type", "Route"},
                              {"from", "stop 1"},
                              {"to", "stop 2"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<to> isn't string",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1},
                              {"to", 2},
                              {"from", "1"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<id> isn't int",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1.2},
                              {"to", "2"},
                              {"from", "1"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<from> isn't string",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1},
                              {"from", 1},
                              {"to", "2"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Common request",
          .input = json::Dict{{"type", "Route"},
                              {"from", "stop 1"},
                              {"to", "stop 2"},
                              {"id", 4}},
          .want = GetRouteRequest{
              .id = 4,
              .from = "stop 1",
              .to = "stop 2"}
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseGetRouteRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestOutputRequest, TestGetStopRequest) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<GetStopRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <name>",
          .input = json::Dict{{"type", "Stop"},
                              {"id", 1965312327}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: no <id>",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "Stop1"}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong request: <id> isn't int",
          .input = json::Dict{{"type", "Route"},
                              {"from", "stop 1"},
                              {"to", "stop 2"},
                              {"id", 12551.21}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "<name> isn't string",
          .input = json::Dict{{"type", "Bus"},
                              {"id", 1},
                              {"name", 1}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Common request",
          .input = json::Dict{{"type", "Stop"},
                              {"name", "Stop1"},
                              {"id", 17289739}},
          .want = GetStopRequest{
              .id = 17289739,
              .stop = "Stop1",}
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseGetStopRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestOutputRequest, TestGetMapRequest) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<GetMapRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <id>",
          .input = json::Dict{{"type", "Map"}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: <id> isn't int",
          .input = json::Dict{{"type", "Map"},
                              {"id", "4251"}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Common request",
          .input = json::Dict{{"type", "Map"},
                              {"id", 83157}},
          .want = GetMapRequest{.id = 83157}
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseGetMapRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

struct RequestCount {
  int bus = 0;
  int stop = 0;
  int route = 0;
  int map = 0;

  friend bool operator==(const RequestCount lhs, const RequestCount rhs) {
    return std::tie(lhs.bus, lhs.stop, lhs.route, lhs.map)
        == std::tie(rhs.bus, rhs.stop, rhs.route, rhs.map);
  }
};

TEST(TestOutput, TestOutputRequestsCount) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::List input;
    std::optional<RequestCount> want;
  };
  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Not found key \"stat_requests\"",
          .input = json::List{{"base_requests",
                               json::List{json::Dict{{"type", "Bus"},
                                                     {"name", "Bus 1"},
                                                     {"id", 5434637}},
                                          json::Dict{{"type", "Bus"},
                                                     {"name", "Bus 2"},
                                                     {"id", 98336215}},
                                          json::Dict{{"type", "Bus"},
                                                     {"name", "Bus 3"},
                                                     {"id", 473829}},
                                          json::Dict{{"type", "Bus"},
                                                     {"name", "Bus 4"}},
                                          json::Dict{{"type", "Route"},
                                                     {"from", "stop1"},
                                                     {"to", "stop3"},
                                                     {"id", 3423626}},
                                          json::Dict{{"type", "Route"},
                                                     {"from", "stop4"},
                                                     {"to", "stop2"}},
                                          json::Dict{{"type", "Bus"},
                                                     {"name", "Bus 4"}},
                                          json::Dict{{"type", "Stop"},
                                                     {"name", "Stop 1"},
                                                     {"id", 635478}},
                                          json::Dict{{"type", "Stop"},
                                                     {"name", "Stop 2"}}}}},
          .want = std::nullopt,
      },
      TestCase{
          .name = "Test requests count",
          .input = json::List{json::Dict{{"type", "Bus"},
                                         {"name", "Bus 1"},
                                         {"id", 5434637}},
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 2"},
                                         {"id", 98336215}},
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 3"},
                                         {"id", 473829}},
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 4"}},
                              json::Dict{{"type", "Route"},
                                         {"from", "stop1"},
                                         {"to", "stop3"},
                                         {"id", 3423626}},
                              json::Dict{{"type", "Route"},
                                         {"from", "stop2"},
                                         {"to", "stop7"},
                                         {"id", 952790}},
                              json::Dict{{"type", "Map"},
                                         {"id", 3521}},
                              json::Dict{{"type", "Map"},
                                         {"id", "3820"}},
                              json::Dict{{"type", "Route"},
                                         {"from", "stop4"},
                                         {"to", "stop2"}},
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 4"}},
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 1"},
                                         {"id", 635478}},
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 2"}}},
          .want = {RequestCount{.bus = 3, .stop= 1, .route= 2, .map = 1}},
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got_opt = ParseOutput(input);
    if (!got_opt) {
      EXPECT_FALSE(want) << name;
      continue;
    }
    int bus_req_count = count_if(got_opt->begin(), got_opt->end(),
                                 [](const GetRequest &req) {
                                   return std::holds_alternative<GetBusRequest>(
                                       req);
                                 });
    int stop_req_count = count_if(got_opt->begin(), got_opt->end(),
                                  [](const GetRequest &req) {
                                    return std::holds_alternative<GetStopRequest>(
                                        req);
                                  });
    int route_req_count = count_if(
        got_opt->begin(), got_opt->end(), [](const GetRequest &req) {
          return std::holds_alternative<GetRouteRequest>(req);
        });
    int map_req_count = count_if(
        got_opt->begin(), got_opt->end(), [](const GetRequest &req) {
          return std::holds_alternative<GetMapRequest>(req);
        });
    auto got = RequestCount{.bus = bus_req_count, .stop = stop_req_count,
        .route = route_req_count, .map = map_req_count};
    EXPECT_EQ(*want, got) << name;
  }
}

TEST(TestInput, TestInputRequestsCount) {
  using namespace rm;
  struct TestCase {
    std::string name;
    json::List input;
    std::optional<std::pair<int, int>> want;
  };
  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Test requests count",
          .input = json::List{json::Dict{{"type", "Bus"},
                                         {"name", "Bus 1"},
                                         {"stops", json::List{
                                             "stop1", "stop3",
                                             "stop2", "stop1"}},
                                         {"is_roundtrip", true}},
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 2"},
                                         {"stops", json::List{
                                             "stop2", "stop3"}},
                                         {"is_roundtrip", false}
                              },
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 3"},
                                         {"is_roundtrip", false}
                              },
                              json::List{"type", "Bus", "name", "Bus 3",
                                         "is_roundtrip", false},
          },
          .want = std::nullopt,
      },
      TestCase{
          .name = "Test requests count",
          .input = json::List{json::Dict{{"type", "Bus"},
                                         {"name", "Bus 1"},
                                         {"stops", json::List{
                                             "stop1", "stop3",
                                             "stop2", "stop1"}},
                                         {"is_roundtrip", true}
          },
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 2"},
                                         {"stops", json::List{
                                             "stop2", "stop3"}},
                                         {"is_roundtrip", false}
                              },
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 2"},
                                         {"stops", json::List{}},
                                         {"is_roundtrip", false}
                              },
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 3"},
                                         {"is_roundtrip", false}
                              },
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 1"},
                                         {"latitude", 36.637519},
                                         {"longitude", 138.267031},
                                         {"road_distances", json::Dict{
                                             {"Stop 2", 18470},
                                             {"Stop 3", 83690}
                                         }}},
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 2"},
                                         {"latitude", 36.637519},
                                         {"road_distances", json::Dict{
                                             {"Stop 4", 20570},
                                             {"Stop 1", 57192}
                                         }}},
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 2"},
                                         {"latitude", 36.637519},
                                         {"road_distances", json::Dict{
                                             {"Stop 4", 20570},
                                             {"Stop 1", 57192}
                                         }}}},
          .want = std::pair<int, int>{2, 1},
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got_opt = ParseInput(input);
    if (!got_opt) {
      EXPECT_FALSE(want) << name;
      continue;
    }
    int bus_req_count = count_if(got_opt->begin(), got_opt->end(),
                                 [](const PostRequest &req) {
                                   return std::holds_alternative<PostBusRequest>(
                                       req);
                                 });
    int stop_req_count = count_if(got_opt->begin(), got_opt->end(),
                                  [](const PostRequest &req) {
                                    return std::holds_alternative<
                                        PostStopRequest>(
                                        req);
                                  });
    std::pair got = {bus_req_count, stop_req_count};
    EXPECT_EQ(want, got) << name;
  }
}
