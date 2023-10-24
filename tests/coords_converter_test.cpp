#include <string>
#include <vector>
#include <utility>

#include "gtest/gtest.h"

#include "src/coords_converter.h"
#include "test_utils.h"

using namespace std;

const std::pair<std::string_view, rm::sphere::Coords> kAirport =
    {"Airport", {.latitude = 12.312245, .longitude = 52.119401}};
const std::pair<std::string_view, rm::sphere::Coords> kShop =
    {"Shop", {.latitude = 73.132015, .longitude = -46.132028}};
const std::pair<std::string_view, rm::sphere::Coords> kHighStreet =
    {"High Street", {.latitude = 42.111111, .longitude = 19.291527}};
const std::pair<std::string_view, rm::sphere::Coords> kRWStation =
    {"RW station", {.latitude = 63.015232, .longitude = -35.621022}};
const std::pair<std::string_view, rm::sphere::Coords> kClemens =
    {"Clemens street", {.latitude = 49.102842, .longitude = 39.845290}};

TEST(TestSortStops, TestSortByLatitude) {
  struct TestCase {
    string name;
    rm::renderer_utils::Stops stops;
    vector<string_view> want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty names",
          .stops = {},
          .want = {},
      },
      TestCase{
          .name = "Stop count == 1",
          .stops = {kAirport},
          .want = {"Airport"},
      },
      TestCase{
          .name = "Stop count > 1",
          .stops = {kAirport, kShop, kHighStreet},
          .want = {"Airport", "High Street", "Shop"},
      },
  };

  for (auto &[name, stops, want] : test_cases) {
    auto got = SortStops(stops, rm::coords_converter::SortMode::kByLatitude);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestSortStops, TestSortByLongitude) {
  struct TestCase {
    string name;
    rm::renderer_utils::Stops stops;
    vector<string_view> want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty names",
          .stops = {},
          .want = {},
      },
      TestCase{
          .name = "Stop count == 1",
          .stops = {kAirport},
          .want = {"Airport"},
      },
      TestCase{
          .name = "Stop count > 1",
          .stops = {kAirport, kShop, kHighStreet},
          .want = {"Shop", "High Street", "Airport"},
      },
  };

  for (auto &[name, stops, want] : test_cases) {
    auto got = SortStops(stops, rm::coords_converter::SortMode::kByLongitude);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestAdjacentStops, TestAdjacentStops) {
  struct TestCase {
    string name;
    rm::renderer_utils::Buses buses;
    vector<pair<string_view, string_view>> want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty buses",
          .want = {},
      },
      TestCase{
          .name = "Non-empty buses",
          .buses = {{"1", {{"1", "2", "3"}, false}},
                    {"2", {{"3", "4", "5", "3"}, true}}},
          .want = {{"1", "2"}, {"2", "3"}, {"3", "4"}, {"4", "5"}, {"5", "3"}},
      },
      TestCase{
          .name = "Repeated stop",
          .buses = {{"1", {{"1", "2", "2", "3"}, true}}},
          .want = {{"1", "2"}, {"2", "3"}},
      },
  };

  for (auto &[name, buses, want] : test_cases) {
    auto got = rm::coords_converter::AdjacentStops(buses);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestCompressCoords, TestCompressCoords) {
  struct TestCase {
    string name;
    vector<string_view> stops;
    vector<pair<string_view, string_view>> neighbors;
    vector<vector<string_view>> want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty stops",
          .want = {},
      },
      TestCase{
          .name = "Empty neighbors",
          .stops = {"3", "1", "4", "2"},
          .want = {{"3", "1", "4", "2"}},
      },
      TestCase{
          .name = "1 stop",
          .stops = {"3"},
          .neighbors = {{"1", "2"}},
          .want = {{"3"}},
      },
      TestCase{
          .name = "Basic functionality",
          .stops = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"},
          .neighbors = {{"4", "5"}, {"7", "8"}, {"9", "10"}, {"2", "5"},
                        {"1", "9"}, {"4", "8"}},
          .want = {{"1", "2", "3", "4"}, {"5", "6", "7"}, {"8", "9"}, {"10"}},
      }
  };

  for (auto &[name, stops, neighbors, want] : test_cases) {
    auto got = rm::coords_converter::CompressNonadjacent(stops, neighbors);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestSpreadStops, TestSpreadStops) {
  struct TestCase {
    string name;
    vector<vector<string_view>> layers;
    double from;
    double to;
    vector<pair<string_view, double>> want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty layers",
          .layers = {},
          .from = 100,
          .to = 400,
          .want = {},
      },
      TestCase{
          .name = "from > to",
          .layers = {{"3", "1", "2"}},
          .from = 300,
          .to = 100,
          .want = {{"3", 300}, {"1", 300}, {"2", 300}},
      },
      TestCase{
          .name = "to > from",
          .layers = {{"3", "1", "2"}},
          .from = 100,
          .to = 300,
          .want = {{"3", 100}, {"1", 100}, {"2", 100}},
      },
      TestCase{
          .name = "to == from",
          .layers = {{"3", "1", "2"}},
          .from = 100,
          .to = 100,
          .want = {{"3", 100}, {"1", 100}, {"2", 100}},
      },
      TestCase{
          .name = "from > to",
          .layers = {{"3"}, {"1"}, {"2", "4"}},
          .from = 300,
          .to = 100,
          .want = {{"3", 300}, {"1", 200}, {"2", 100}, {"4", 100}},
      },
      TestCase{
          .name = "to > from",
          .layers = {{"3", "4", "5"}, {"1"}, {"2"}},
          .from = 100,
          .to = 300,
          .want = {{"3", 100}, {"4", 100}, {"5", 100}, {"1", 200}, {"2", 300}},
      },
      TestCase{
          .name = "to == from",
          .layers = {{"3", "4", "5"}, {"1"}, {"2"}},
          .from = 100,
          .to = 100,
          .want = {{"3", 100}, {"4", 100}, {"5", 100}, {"1", 100}, {"2", 100}},
      },
  };

  for (auto &[name, layers, from, to, want] : test_cases) {
    auto got = rm::coords_converter::SpreadStops(layers, from, to);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestEndPoints, TestEndPoints) {
  struct TestCase {
    std::string name;
    rm::renderer_utils::Buses buses;
    std::unordered_set<std::string_view> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Roundtrip",
          .buses = {{"bus1", {{"a", "b", "c", "a"}, true}}},
          .want = {"a"}
      },
      TestCase{
          .name = "Non-roundtrip with different end points",
          .buses = {{"bus1", {{"a", "b", "c", "d"}, false}}},
          .want = {"a", "d"}
      },
      TestCase{
          .name = "Non-roundtrip with the same end points",
          .buses = {{"bus1", {{"a", "b", "c", "a"}, false}}},
          .want = {"a"}
      },
  };

  for (auto &[name, buses, want] : test_cases) {
    auto got = rm::coords_converter::EndPoints(buses);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestIntersectionsWithinRoute, TestIntersectionsWithinRoute) {
  struct TestCase {
    std::string name;
    rm::renderer_utils::Buses buses;
    struct Subcase {
      int count;
      std::unordered_set<std::string_view> want;
    };
    vector<Subcase> subcases;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Base cases",
          .buses = {{"bus1", {{"a", "b", "c", "b", "a", "b", "a"}, true}},
                    {"bus2", {{"d", "e", "f", "g", "e"}, false}},
                    {"bus3", {{"h", "i", "j", "k"}, false}},
                    {"bus4", {{"l", "m", "n", "o", "l"}, true}}},
          .subcases = {
              {.count = 1, .want = {"a", "b", "c", "d", "e", "f", "g", "h", "i",
                                    "j", "k", "l", "m", "n", "o"}},
              {.count = 2, .want = {"a", "b", "d", "e", "f", "g", "h", "i",
                                    "j", "l"}},
              {.count = 3, .want = {"a", "b", "e"}},
              {.count = 4, .want = {}}},
      },
  };

  for (auto &[name, buses, subcases] : test_cases) {
    for (auto &subcase : subcases) {
      auto got = rm::coords_converter::IntersectionsWithinRoute(buses,
                                                                subcase.count);
      auto &want = subcase.want;
      EXPECT_EQ(want, got) << name << ". Count = " << subcase.count;
    }
  }
}

TEST(TestIntersectionsCrossRoute, TestIntersectionsCrossRoute) {
  struct TestCase {
    std::string name;
    rm::renderer_utils::Buses buses;
    std::unordered_set<std::string_view> want;
  };

  vector<TestCase> test_cases = {
      TestCase{
          .name = "Base request",
          .buses = {
              {"bus1", {{"a", "b", "c", "a"}, true}},
              {"bus2", {{"d", "f", "c", "g"}, false}},
              {"bus3", {{"k", "b", "c", "f"}, false}},
              {"bus4", {{"x", "y", "z"}, false}}},
          .want = {"b", "c", "f"}
      },
      TestCase{
          .name = "Empty routes",
          .buses = {
              {"bus1", {{}, true}},
              {"bus2", {{}, false}}
          },
          .want = {}
      }
  };

  for (auto &[name, buses, want] : test_cases) {
    auto got = rm::coords_converter::IntersectionsCrossRoute(buses);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestInterpolation, TestInterpolation) {
  struct TestCase {
    string name;
    vector<string_view> route;
    unordered_set<string_view> base_stops;
    rm::renderer_utils::Stops stops;
    rm::renderer_utils::Stops want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "1 stops between base_stops",
          .route = {"1", "2", "3", "4", "3", "2", "1"},
          .base_stops = {"1", "2", "4"},
          .stops = {{"1", {10, 10}}, {"2", {20, 20}}, {"3", {25, 25}},
                    {"4", {40, 0}}},
          .want = {{"1", {10, 10}}, {"2", {20, 20}}, {"3", {30, 10}},
                   {"4", {40, 0}}}
      },
      TestCase{
          .name = "Several stops between base_stops",
          .route = {"1", "2", "3", "4", "3", "2", "1"},
          .base_stops = {"1", "4"},
          .stops = {{"1", {10, 30}}, {"2", {20, 20}}, {"3", {25, 25}},
                    {"4", {40, 0}}},
          .want = {{"1", {10, 30}}, {"2", {20, 20}}, {"3", {30, 10}},
                   {"4", {40, 0}}}
      },
      TestCase{
          .name = "Several interpolation possibilities",
          .route = {"1", "2", "3", "4", "3", "5", "1"},
          .base_stops = {"1", "3", "4", "5"},
          .stops = {{"1", {10, 10}}, {"2", {15, 15}}, {"3", {25, 25}},
                    {"4", {30, 10}}, {"5", {10, 30}}},
          .want = {{"1", {10, 10}}, {"2", {17.5, 17.5}}, {"3", {25, 25}},
                   {"4", {30, 10}}, {"5", {10, 30}}},
      },
  };

  for (auto &[name, route, base_stops, stops, want] : test_cases) {
    auto got = rm::coords_converter::Interpolate(std::move(stops), route,
                                                 base_stops);
    EXPECT_EQ(want, got) << name;
  }
}
