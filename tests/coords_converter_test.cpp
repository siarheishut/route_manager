#include <string>
#include <vector>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

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
    rm::coords_converter::AdjacentList want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty buses",
          .want = {},
      },
      TestCase{
          .name = "One bus",
          .buses = {{"bus 1", {{"a", "b", "c", "d", "a"}, true}}},
          .want = {{"a", {"b", "d"}}, {"b", {"a", "c"}}, {"c", {"b", "d"}},
                   {"d", {"a", "c"}}}
      },
      TestCase{
          .name = "Repeated stop",
          .buses = {{"bus 1", {{"a", "a", "c", "d", "a"}, true}}},
          .want = {{"a", {"c", "d"}}, {"c", {"a", "d"}}, {"d", {"a", "c"}}}
      },
      TestCase{
          .name = "Several buses",
          .buses = {{"bus 1", {{"a", "b", "c", "d", "a"}, true}},
                    {"bus 2", {{"e", "f", "b", "g", "h"}, false}},
                    {"bus 3", {{"a", "i", "g", "j", "b"}, false}}},
          .want = {{"a", {"b", "d", "i"}}, {"b", {"a", "c", "f", "g", "j"}},
                   {"c", {"b", "d"}}, {"d", {"a", "c"}}, {"e", {"f"}},
                   {"f", {"b", "e"}}, {"g", {"b", "h", "i", "j"}}, {"h", {"g"}},
                   {"i", {"a", "g"}}, {"j", {"b", "g"}}},
      }
  };

  for (auto &[name, buses, want] : test_cases) {
    auto got = rm::coords_converter::AdjacentStops(buses);
    EXPECT_EQ(want, got) << name;
  }
}

MATCHER(WhenSortedEq, "") {
  auto v1 = get<0>(arg);
  auto v2 = get<1>(arg);
  sort(begin(v1), end(v1));
  sort(begin(v2), end(v2));
  return v1 == v2;
}

TEST(TestCompressStops, TestCompressStops) {
  struct TestCase {
    string name;
    vector<string_view> stops;
    rm::coords_converter::AdjacentList adj_stops;
    rm::coords_converter::StopLayers want;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Empty stops",
          .adj_stops =  {{"a", {"b", "c"}}, {"b", {"a", "c"}},
                         {"c", {"a", "b"}}},
          .want = {},
      },
      TestCase{
          .name = "All stops on one layer",
          .stops = {"a", "b", "d"},
          .adj_stops = {},
          .want = {{"a", "b", "d"}},
      },
      TestCase{
          .name = "One stop on each layer",
          .stops = {"a", "b", "c", "d"},
          .adj_stops = {{"a", {"b", "c", "d"}}, {"b", {"a", "c", "d"}},
                        {"c", {"a", "d", "b"}}, {"d", {"a", "b", "c"}}},
          .want = {{"a"}, {"b"}, {"c"}, {"d"}}
      },
      TestCase{
          .name = "Stop is not in adjacent_list",
          .stops = {"a", "b", "d"},
          .adj_stops = {{"a", {"b", "c"}}, {"b", {"a", "c"}},
                        {"c", {"a", "b"}}},
          .want = {{"a", "d"}, {"b"}},
      },
  };

  for (auto &[name, stops, adj_list, want] : test_cases) {
    auto got = rm::coords_converter::CompressNonadjacent(stops, adj_list);

    using namespace ::testing;
    EXPECT_THAT(want, Pointwise(WhenSortedEq(), got)) << name;
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
