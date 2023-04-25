#include <gtest/gtest.h>

#include "src/json/json.h"
#include "src/request_parser.h"
#include "test_utils.h"

#include <string>
#include <variant>
#include <vector>

TEST(TestInputRequest, TestPostBusRequest) {
  using namespace rm;

  struct TestCase {
    std::string name;
    rm::json::Dict input;
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
              .stops = {"stop1", "stop2", "stop3", "stop1"}
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
              .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"}
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
              .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"}
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
              .stops = {"stop1", "stop2", "stop1"}
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
              .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"},
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
          },
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
    rm::json::Dict input;
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
    rm::json::Dict input;
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

TEST(TestOutputRequest, TestGetStopRequest) {
  using namespace rm;

  struct TestCase {
    std::string name;
    rm::json::Dict input;
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

TEST(TestParseRequests, TestOutput) {
  using namespace rm;

  struct TestCase {
    std::string name;
    rm::json::List input;
    std::optional<std::pair<int, int>> want;
  };
  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Not found key \"stat_requests\"",
          .input = rm::json::List{{"base_requests",
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
                              json::Dict{{"type", "Bus"},
                                         {"name", "Bus 4"}},
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 1"},
                                         {"id", 635478}},
                              json::Dict{{"type", "Stop"},
                                         {"name", "Stop 2"}}},
          .want = {{3, 1}},
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
    std::pair<int, int> got = {bus_req_count, stop_req_count};
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestParseRequests, TestInput) {
  using namespace rm;
  struct TestCase {
    std::string name;
    rm::json::List input;
    std::optional<std::pair<int, int>> want;
  };
  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Test requests count",
          .input = rm::json::List{json::Dict{{"type", "Bus"},
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
