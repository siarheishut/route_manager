#include "src/transport_catalog.h"

#include <string>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"

#include "src/common.h"
#include "src/request_types.h"
#include "src/sphere.h"
#include "test_utils.h"
#include "transport_catalog.pb.h"

using namespace std;
using namespace rm::utils;

namespace {
auto CommonPostRequests() {
  return std::vector<rm::utils::PostRequest>{
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
          .coords = {1, 1},
          .stop_distances = {{"stop2", 160000}}},
      PostStopRequest{
          .stop = "stop2",
          .coords = {2, 2},
          .stop_distances = {{"stop3", 160000}}},
      PostStopRequest{
          .stop = "stop3",
          .coords = {3, 3},
          .stop_distances = {{"stop4", 160000}, {"stop7", 640000},
                             {"stop8", 800000}}},
      PostStopRequest{
          .stop = "stop4",
          .coords = {4, 4},
          .stop_distances = {{"stop5", 160000}}},
      PostStopRequest{
          .stop = "stop5",
          .coords = {5, 5},
          .stop_distances = {{"stop6", 160000}}},
      PostStopRequest{
          .stop = "stop6",
          .coords = {6, 6},
          .stop_distances = {{"stop7", 160000}}},
      PostStopRequest{
          .stop = "stop7",
          .coords = {7, 7}},
      PostStopRequest{
          .stop = "stop8",
          .coords = {8, 8}},
  };
}

struct ProtoStop {
  std::string name;
  rm::sphere::Coords coords;

  ::TransportCatalog::Stop Build() {
    ::TransportCatalog::Stop stop;
    stop.set_stop_name(name);
    stop.mutable_coords()->set_latitude(coords.latitude);
    stop.mutable_coords()->set_longitude(coords.longitude);
    return stop;
  }
};

struct ProtoBus {
  std::string name;
  double distance;
  double curvature;
  std::vector<int> route;
  std::unordered_set<int> endpoints;

  ::TransportCatalog::Bus Build() {
    ::TransportCatalog::Bus bus;
    bus.set_bus_name(name);
    bus.set_length(static_cast<int>(distance));
    bus.set_curvature(curvature);
    *bus.mutable_route() = {route.begin(), route.end()};
    *bus.mutable_endpoints() = {endpoints.begin(), endpoints.end()};
    return bus;
  }
};
}

TEST(TestTransportCatalog, TestDeserializationFactory) {
  using namespace rm;

  struct TestCase {
    std::string name;
    ::TransportCatalog::TransportDatabase config;
    bool want_fail;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Bus redefinition",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop2", {2, 2}}.Build();
            *database.add_stops() = ProtoStop{"stop3", {3, 3}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "Bus1",
                .distance = 640000,
                .curvature = 1.0178,
                .route = {0, 1, 2, 1, 0},
                .endpoints = {0, 2}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "Bus1",
                .distance = 640000,
                .curvature = 1.0178,
                .route = {0, 1, 2, 0},
                .endpoints = {0}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Stop redefinition",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop1", {2, 2}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Unknown stop along the route",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop2", {2, 2}}.Build();
            *database.add_stops() = ProtoStop{"stop3", {3, 3}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "Bus1",
                .distance = 640000,
                .curvature = 1.0178,
                .route = {0, 1, 8, 1, 0},
                .endpoints = {0}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Unknown stop among endpoints",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop2", {2, 2}}.Build();
            *database.add_stops() = ProtoStop{"stop3", {3, 3}}.Build();
            ProtoBus bus{
                .name = "Bus1",
                .distance = 640000,
                .curvature = 1.0178,
                .route = {0, 1, 2, 1, 0},
                .endpoints = {0, 8}};
            *database.add_buses() = bus.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Latitude out of range(above)",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"new_stop", {91, 0}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Longitude out of range(above)",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"new_stop", {0, 181}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Latitude out of range(below)",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"new_stop", {-91, 0}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Longitude out of range(below)",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() =
                ProtoStop{"new_stop", {0, -181}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Stop count in a route < 3",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "Bus1",
                .distance = 0,
                .curvature = 1.0,
                .route = {0, 0},
                .endpoints = {0}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "End stop != start stop",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop2", {2, 2}}.Build();
            *database.add_stops() = ProtoStop{"stop3", {3, 3}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "NewBus",
                .distance = 320000,
                .curvature = 1.0178,
                .route = {0, 1, 2},
                .endpoints = {0}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Endpoint isn't within the route",
          .config = [&] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop2", {2, 2}}.Build();
            *database.add_stops() = ProtoStop{"stop3", {3, 3}}.Build();
            *database.add_stops() = ProtoStop{"stop4", {4, 4}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "Bus1",
                .distance = 640000,
                .curvature = 1.0178,
                .route = {0, 1, 2, 1, 0},
                .endpoints = {0, 3}}.Build();
            return database;
          }(),
          .want_fail = true,
      },
      TestCase{
          .name = "Correct data",
          .config = [] {
            ::TransportCatalog::TransportDatabase database;
            *database.add_buses() = ProtoBus{
                .name = "Bus1",
                .distance = 640000,
                .curvature = 1.0178,
                .route = {0, 1, 2, 1, 0},
                .endpoints = {0, 2}}.Build();
            *database.add_buses() = ProtoBus{
                .name = "Bus2",
                .distance = 1280000,
                .curvature = 1.01951,
                .route = {2, 3, 4, 5, 6, 2},
                .endpoints = {2}}.Build();
            *database.add_stops() = ProtoStop{"stop1", {1, 1}}.Build();
            *database.add_stops() = ProtoStop{"stop2", {2, 2}}.Build();
            *database.add_stops() = ProtoStop{"stop3", {3, 3}}.Build();
            *database.add_stops() = ProtoStop{"stop4", {4, 4}}.Build();
            *database.add_stops() = ProtoStop{"stop5", {5, 5}}.Build();
            *database.add_stops() = ProtoStop{"stop6", {6, 6}}.Build();
            *database.add_stops() = ProtoStop{"stop7", {7, 7}}.Build();
            return database;
          }(),
          .want_fail = false,
      },
  };

  for (auto &[name, proto_database, want_fail] : test_cases) {
    auto failed = !rm::TransportCatalog::Deserialize(proto_database);
    EXPECT_EQ(want_fail, failed) << name;
  }
}

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
    bool got = rm::TransportCatalog::Create(config) == nullptr;
    EXPECT_EQ(want_fail, got) << name;
  }
}

TEST(TestTransportCatalog, TestSerializationAndDeserialization) {
  auto initial = rm::TransportCatalog::Create(CommonPostRequests());
  ASSERT_TRUE(initial) << "Initial item wasn't created.";
  ::TransportCatalog::TransportDatabase proto_database;
  proto_database = initial->Serialize();
  auto deserialized = rm::TransportCatalog::Deserialize(proto_database);
  ASSERT_TRUE(deserialized) << "Deserialization failed";
  EXPECT_EQ(deserialized->Buses(), initial->Buses())
            << "Invalid buses after deserialization.";
  EXPECT_EQ(deserialized->Stops(), initial->Stops())
            << "Invalid stops after deserialization.";
}

TEST(TestTransportCatalog, TestInitializing) {
  using namespace rm;

  struct TestCase {
    string name;
    vector<PostRequest> config;
    BusDict want_buses;
    StopDict want_stops;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Requests to an empty database",
          .config = {}
      },
      TestCase{
          .name = "Common requests",
          .config = CommonPostRequests(),
          .want_buses = {
              {"Bus1", BusInfo{
                  .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"},
                  .endpoints = {"stop1", "stop3"},
                  .unique_stop_count = 3,
                  .distance = 640000,
                  .curvature = 1.0178}},
              {"Bus2", BusInfo{
                  .stops = {"stop3", "stop4", "stop5", "stop6", "stop7",
                            "stop3"},
                  .endpoints = {"stop3"},
                  .unique_stop_count = 5,
                  .distance = 1280000,
                  .curvature = 1.01951}}
          },
          .want_stops = {
              {"stop1", StopInfo{
                  .dists = {{"stop2", 160000}},
                  .coords = {1, 1},
                  .buses = {"Bus1"}}},
              {"stop2", StopInfo{
                  .dists = {{"stop3", 160000}},
                  .coords = {2, 2},
                  .buses = {"Bus1"}}},
              {"stop3", StopInfo{
                  .dists = {{"stop4", 160000}, {"stop7", 640000},
                            {"stop8", 800000}},
                  .coords = {3, 3},
                  .buses = {"Bus1", "Bus2"}}},
              {"stop4", StopInfo{
                  .dists = {{"stop5", 160000}},
                  .coords = {4, 4},
                  .buses = {"Bus2"}}},
              {"stop5", StopInfo{
                  .dists = {{"stop6", 160000}},
                  .coords = {5, 5},
                  .buses = {"Bus2"}}},
              {"stop6", StopInfo{
                  .dists = {{"stop7", 160000}},
                  .coords = {6, 6},
                  .buses = {"Bus2"}}},
              {"stop7", StopInfo{
                  .coords = {7, 7},
                  .buses = {"Bus2"}}},
              {"stop8", StopInfo{
                  .coords = {8, 8},
                  .buses = {}}},
          }
      },
  };

  for (auto &[name, config, want_buses, want_stops] : test_cases) {
    auto tc = rm::TransportCatalog::Create(config);
    EXPECT_TRUE(tc) << name;
    if (!tc) continue;

    auto got_stops = tc->Stops();
    auto got_buses = tc->Buses();

    EXPECT_EQ(want_buses, got_buses) << name;
    EXPECT_EQ(want_stops, got_stops) << name;
  }
}
