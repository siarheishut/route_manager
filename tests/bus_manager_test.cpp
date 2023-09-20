#include "src/bus_manager.h"

#include <optional>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "test_utils.h"

const rm::RoutingSettings kTestRoutingSettings{
    .bus_wait_time = 7,
    .bus_velocity = 54.0
};

TEST(TestFactoryMethod, TestInitializing) {
  using namespace rm;

  struct TestCase {
    std::string name;
    std::vector<PostRequest> config;
    RoutingSettings routing_settings;
    bool want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Bus redefinition",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 2", "stop 3", "stop 1", "stop 2"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 10000}, {"stop 3", 20000}}
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 3", 7000}},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = false,
      },
      TestCase{
          .name = "Fixed \"Bus redefinition\"",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostBusRequest{
                  .bus = "Bus 2",
                  .stops = {"stop 2", "stop 3", "stop 1", "stop 2"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 10000}, {"stop 3", 20000}}
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 3", 7000}},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = true,
      },
      TestCase{
          .name = "Stop redefinition",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {44.444444, 44.444444},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = false,
      },
      TestCase{
          .name = "Fixed \"Stop redefinition\"",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
              },
              PostStopRequest{
                  .stop = "stop 4",
                  .coords = {44.444444, 44.444444},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = true,
      },
      TestCase{
          .name = "Unknown stop along the route",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 4", "stop 1"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = false,
      },
      TestCase{
          .name = "Fixed \"Unknown stop along the route\"",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 4", "stop 1"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
              },
              PostStopRequest{
                  .stop = "stop 4",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = true,
      },
      TestCase{
          .name = "Unknown stop among road_distances",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 2000},},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 4", 5000}},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
                  .stop_distances = {{"stop 1", 4000}},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = false,
      },
      TestCase{
          .name = "Fixed \"Unknown stop among road_distances\"",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 2000},},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 3", 5000}},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
                  .stop_distances = {{"stop 1", 4000}},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = true,
      },
      TestCase{
          .name = "Unused stop",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 2000},},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 3", 2000},},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
                  .stop_distances = {{"stop 4", 5000}},
              },
              PostStopRequest{
                  .stop = "stop 4",
                  .coords = {44.444444, 44.444444},
                  .stop_distances = {{"stop 1", 4000}},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = true,
      },
      TestCase{
          .name = "Empty route",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 2000},},
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 3", 2000},},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = false,
      },
      TestCase{
          .name = "Correct data",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostBusRequest{
                  .bus = "Bus 2",
                  .stops = {"stop 2", "stop 3", "stop 1", "stop 2"},
              },
              PostBusRequest{
                  .bus = "Bus 3",
                  .stops = {"stop 3", "stop 2", "stop 1", "stop 3"},
              },
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 10000}, {"stop 3", 20000}}
              },
              PostStopRequest{
                  .stop = "stop 2",
                  .coords = {22.222222, 22.222222},
                  .stop_distances = {{"stop 3", 7000}},
              },
              PostStopRequest{
                  .stop = "stop 3",
                  .coords = {33.333333, 33.333333},
              },
          },
          .routing_settings = kTestRoutingSettings,
          .want = true,
      },
  };

  for (auto &[name, config, routing_settings, want] : test_cases) {
    bool got = BusManager::Create(config, routing_settings) != nullptr;
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestBusManager, TestGetBusInfo) {
  using namespace rm;

  struct TestCase {
    std::string name;
    std::vector<PostRequest> config;
    RoutingSettings routing_settings;
    std::vector<GetBusRequest> requests;
    std::vector<std::optional<BusResponse>> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {},
          .routing_settings = kTestRoutingSettings,
          .requests = {GetBusRequest{.bus = "123"},
                       GetBusRequest{.bus = "some bus"},
                       GetBusRequest{.bus = "s      s"},},
          .want = {std::nullopt,
                   std::nullopt,
                   std::nullopt},
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
                  .stop_distances = {{"stop2", 3000}}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.595884, 37.209755}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {55.632761, 37.333324}},
              PostStopRequest{
                  .stop = "stop4",
                  .coords = {55.574371, 37.6517},
                  .stop_distances = {{"stop5", 4000}, {"stop3", 4000}}},
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
          .routing_settings = kTestRoutingSettings,
          .requests = {GetBusRequest{.bus = "Bus1"},
                       GetBusRequest{.bus = "Bus2"},
                       GetBusRequest{.bus = "none"}},
          .want = {BusResponse{5, 3, 2.35535e+04},
                   BusResponse{6, 5, 7598.15},
                   std::nullopt},
      },
  };

  for (auto &[name, test_item, routing_settings, requests, want] : test_cases) {
    auto bm = BusManager::Create(test_item, routing_settings);
    EXPECT_TRUE(bm) << name;
    if (!bm) continue;

    for (int i = 0; i < requests.size(); ++i) {
      auto got = bm->GetBusInfo(requests[i].bus);

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
    RoutingSettings routing_settings;
    std::vector<GetStopRequest> requests;
    std::vector<std::optional<StopInfo>> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {},
          .routing_settings = kTestRoutingSettings,
          .requests = {GetStopRequest{.stop = "123"},
                       GetStopRequest{.stop = "some stop"},
                       GetStopRequest{.stop = "s      s"},},
          .want = {std::nullopt,
                   std::nullopt,
                   std::nullopt},
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
          .routing_settings = kTestRoutingSettings,
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

  for (auto &[name, test_item, routing_settings, requests, want] : test_cases) {
    auto bm = BusManager::Create(test_item, routing_settings);
    EXPECT_TRUE(bm) << name;
    if (!bm) continue;

    for (int i = 0; i < requests.size(); ++i) {
      auto got = bm->GetStopInfo(requests[i].stop);

      EXPECT_EQ(want[i].has_value(), got.has_value()) << name;
      if (!got) continue;

      EXPECT_EQ(want[i]->buses, got->buses) << name;
    }
  }
}

TEST(TestBusManager, TestFindRouteInfo) {
  using WaitItem = rm::RouteInfo::WaitItem;
  using RoadItem = rm::RouteInfo::RoadItem;

  using namespace rm;

  struct TestCase {
    std::string name;
    std::vector<PostRequest> config;
    RoutingSettings routing_settings;
    std::vector<GetRouteRequest> requests;
    std::vector<std::optional<RouteResponse>> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {},
          .routing_settings = kTestRoutingSettings,
          .requests = {
              GetRouteRequest{
                  .id = 1,
                  .from = "stop 1",
                  .to = "stop 2"},
              GetRouteRequest{
                  .id = 3,
                  .from = "stop 3",
                  .to = "stop 4"}
          },
          .want = {std::nullopt,
                   std::nullopt},
      },
      // Requests for the paths between existing stops with at least one
      // path between.
      TestCase{
          .name = "Successful requests",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop1"}},
              PostBusRequest{
                  .bus = "Bus2",
                  .stops = {"stop2", "stop3", "stop4", "stop3", "stop2"}},
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {55.574371, 37.6517},
                  .stop_distances = {{"stop2", 2600}}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {55.587655, 37.645687},
                  .stop_distances = {
                      {"stop4", 4650},
                      {"stop2", 1380},
                      {"stop1", 2500}}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.592028, 37.653656},
                  .stop_distances = {{"stop3", 890}}},
              PostStopRequest{
                  .stop = "stop4",
                  .coords = {55.611717, 37.603938},
                  .stop_distances = {}},
          },
          .routing_settings = {.bus_wait_time = 6, .bus_velocity = 40},
          .requests = {
              GetRouteRequest{
                  .id = 1,
                  .from = "stop1",
                  .to = "stop3"
              },
              GetRouteRequest{
                  .id = 2,
                  .from = "stop1",
                  .to = "stop4"
              }
          },
          .want = {
              RouteResponse{
                  .time = 11.235,
                  .items = {
                      WaitItem{.stop = "stop1", .time = 6},
                      RoadItem{.bus = "Bus1", .time = 5.235, .span_count = 2}},
              },
              RouteResponse{
                  .time = 24.21,
                  .items = {
                      WaitItem{.stop = "stop1", .time = 6},
                      RoadItem{.bus = "Bus1", .time = 5.235, .span_count = 2},
                      WaitItem{.stop = "stop3", .time = 6},
                      RoadItem{.bus = "Bus2", .time = 6.975, .span_count = 1},
                  },
              },
          },
      },
      // requests for the paths between mutually unachievable stops.
      TestCase{
          .name = "Failed: unachievable stops",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop1"}},
              PostBusRequest{
                  .bus = "Bus2",
                  .stops = {"stop4", "stop5", "stop6", "stop5", "stop4"}},
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {55.574371, 37.6517},
                  .stop_distances = {{"stop2", 2600}}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {55.587655, 37.645687},
                  .stop_distances = {
                      {"stop2", 1380},
                      {"stop1", 2500}}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.592028, 37.653656},
                  .stop_distances = {{"stop3", 890}}},
              PostStopRequest{
                  .stop = "stop4",
                  .coords = {55.611717, 37.603938},
                  .stop_distances = {}},
              PostStopRequest{
                  .stop = "stop5",
                  .coords = {49.213544, 23.412540},
                  .stop_distances = {}},
              PostStopRequest{
                  .stop = "stop6",
                  .coords = {90.041255, 12.305861},
                  .stop_distances = {}},
          },
          .routing_settings = kTestRoutingSettings,
          .requests = {
              GetRouteRequest{
                  .id = 1,
                  .from = "stop1",
                  .to = "stop4"
              },
              GetRouteRequest{
                  .id = 2,
                  .from = "stop2",
                  .to = "stop5"
              }
          },
          .want = {std::nullopt,
                   std::nullopt},
      },
      // requests for the path between existing stop and missing one.
      TestCase{
          .name = "Failed: missing stop",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop1"}},
              PostBusRequest{
                  .bus = "Bus2",
                  .stops = {"stop2", "stop3", "stop4", "stop3", "stop2"}},
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {55.574371, 37.6517},
                  .stop_distances = {{"stop2", 2600}}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.592028, 37.653656},
                  .stop_distances = {{"stop3", 890}}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {41.841028, 79.832951},
                  .stop_distances = {{"stop4", 914}}},
              PostStopRequest{
                  .stop = "stop4",
                  .coords = {55.611717, 37.603938},
                  .stop_distances = {}},
          },
          .routing_settings = {.bus_wait_time = 6, .bus_velocity = 40},
          .requests = {
              GetRouteRequest{
                  .id = 1,
                  .from = "stop1",
                  .to = "stop5"
              },
              GetRouteRequest{
                  .id = 2,
                  .from = "stop2",
                  .to = "stop6"
              }
          },
          .want = {std::nullopt,
                   std::nullopt},
      },
  };

  for (auto &[name, config, routing_settings, requests, want] : test_cases) {
    auto bm = BusManager::Create(std::move(config), routing_settings);
    EXPECT_TRUE(bm) << name;
    if (!bm) continue;

    for (int i = 0; i < requests.size(); ++i) {
      auto got = bm->GetRoute(requests[i].from, requests[i].to);

      EXPECT_EQ(want[i].has_value(), got.has_value()) << name;
      if (!got || !want[i]) continue;

      EXPECT_EQ(want[i], got) << name;
    }
  }
}
