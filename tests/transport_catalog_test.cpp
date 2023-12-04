#include "src/transport_catalog.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "src/common.h"
#include "src/request_types.h"
#include "test_utils.h"

using namespace std;
using namespace rm::utils;

TEST(TestTransportCatalog, TestFactoryMethod) {
  using namespace rm;

  struct TestCase {
    std::string name;
    std::vector<PostRequest> config;
    bool want_fail;
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
                  .stops = {"stop 2", "stop 3", "stop 1", "stop 3", "stop 2"},
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
          .want_fail = true,
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
          .want_fail = true,
      },
      TestCase{
          .name = "Unknown stop along the route",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 4", "stop 2", "stop 1"},
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
          .want_fail = true,
      },
      TestCase{
          .name = "Unknown stop among road_distances",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {11.111111, 11.111111},
                  .stop_distances = {{"stop 2", 2000}},
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
          .want_fail = true,
      },
      TestCase{
          .name = "Latitude out of range(above)",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {90.1, 0},
              },
          },
          .want_fail = true,
      },
      TestCase{
          .name = "Longitude out of range(above)",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {0, 180.1},
              },
          },
          .want_fail = true,
      },
      TestCase{
          .name = "Latitude out of range(below)",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {-90.1, 0},
              },
          },
          .want_fail = true,
      },
      TestCase{
          .name = "Longitude out of range(below)",
          .config = {
              PostStopRequest{
                  .stop = "stop 1",
                  .coords = {0, -180.1},
              },
          },
          .want_fail = true,
      },
      TestCase{
          .name = "Stop count in a route < 3",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop1"},
                  .endpoints = {"stop1"},
              },
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {0, 0},
              },
          },
          .want_fail = true,
      },
      TestCase{
          .name = "End stop != start stop",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3"},
                  .endpoints = {"stop1", "stop3"},
              },
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {0, 0},
              },
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {1, 1},
              },
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {2, 2},
              },
          },
          .want_fail = true,
      },
      TestCase{
          .name = "Endpoint isn't within the route",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop1"},
                  .endpoints = {"stop1", "stop4"},
              },
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {0, 0},
              },
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {1, 1},
              },
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {2, 2},
              },
              PostStopRequest{
                  .stop = "stop4",
                  .coords = {2, 2},
              },
          },
          .want_fail = true,
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
          .want_fail = false,
      },
      TestCase{
          .name = "Unused stop",
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"stop 1", "stop 2", "stop 3", "stop 1"},
              },
              PostBusRequest{
                  .bus = "Bus 2",
                  .stops = {"stop 2", "stop 1", "stop 3", "stop 1", "stop 2"},
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
          .want_fail = false,
      },
  };

  for (auto &[name, config, want_fail] : test_cases) {
    bool got = TransportCatalog::Create(config) == nullptr;
    EXPECT_EQ(want_fail, got) << name;
  }
}

TEST(TestTransportCatalog, TestInitializing) {
  using namespace rm;

  struct TestCase {
    string name;
    vector<PostRequest> config;
    Buses want_buses;
    Stops want_stops;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {}
      },
      TestCase{
          .name = "Common requests",
          .config = {
              PostBusRequest{
                  .bus = "Bus1",
                  .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"},
                  .endpoints = {"stop1", "stop3"},
              },
              PostBusRequest{
                  .bus = "Bus2",
                  .stops = {"stop3", "stop4", "stop5", "stop6", "stop7",
                            "stop3"},
                  .endpoints = {"stop3"}
              },
              PostStopRequest{
                  .stop = "stop1",
                  .coords = {55.611087, 37.20829}},
              PostStopRequest{
                  .stop = "stop2",
                  .coords = {55.595884, 37.209755},
                  .stop_distances = {{"stop1", 300}}},
              PostStopRequest{
                  .stop = "stop3",
                  .coords = {55.632761, 37.333324},
                  .stop_distances = {{"stop4", 300}, {"stop7", 300},
                                     {"stop8", 600}}},
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
                  .coords = {55.592028, 37.653656},
                  .stop_distances = {{"stop3", 200}}},
              PostStopRequest{
                  .stop = "stop8",
                  .coords = {55.580999, 37.659164}},
          },
          .want_buses = {
              {"Bus1", BusInfo{
                  .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"},
                  .endpoints = {"stop1", "stop3"},
                  .unique_stop_count = 3,
                  .distance = 18153.5,
                  .curvature = 0.86695}},
              {"Bus2", BusInfo{
                  .stops = {"stop3", "stop4", "stop5", "stop6", "stop7",
                            "stop3"},
                  .endpoints = {"stop3"},
                  .unique_stop_count = 5,
                  .distance = 2723.07,
                  .curvature = 0.0620707}}},
          .want_stops = {
              {"stop1", StopInfo{
                  .dists = {{"stop2", 300}},
                  .coords = {55.611087, 37.20829},
                  .buses = {"Bus1"}}},
              {"stop2", StopInfo{
                  .dists = {{"stop1", 300}},
                  .coords = {55.595884, 37.209755},
                  .buses = {"Bus1"}}},
              {"stop3", StopInfo{
                  .dists = {{"stop4", 300}, {"stop7", 300},
                            {"stop8", 600}},
                  .coords = {55.632761, 37.333324},
                  .buses = {"Bus1", "Bus2"}}},
              {"stop4", StopInfo{
                  .dists = {{"stop3", 300}},
                  .coords = {55.574371, 37.6517},
                  .buses = {"Bus2"}}},
              {"stop5", StopInfo{
                  .dists = {},
                  .coords = {55.581065, 37.64839},
                  .buses = {"Bus2"}}},
              {"stop6", StopInfo{
                  .dists = {},
                  .coords = {55.587655, 37.645687},
                  .buses = {"Bus2"}}},
              {"stop7", StopInfo{
                  .dists = {{"stop3", 200}},
                  .coords = {55.592028, 37.653656},
                  .buses = {"Bus2"}}},
              {"stop8", StopInfo{
                  .dists = {{"stop3", 600}},
                  .coords = {55.580999, 37.659164},
                  .buses = {}}},
          }
      },
  };

  for (auto &[name, config, want_buses, want_stops] : test_cases) {
    auto tc = TransportCatalog::Create(config);
    EXPECT_TRUE(tc) << name;
    if (!tc) continue;

    auto got_stops = tc->GetStops();
    auto got_buses = tc->GetBuses();

    EXPECT_EQ(want_buses, got_buses) << name;
    EXPECT_EQ(want_stops, got_stops) << name;
  }
}
