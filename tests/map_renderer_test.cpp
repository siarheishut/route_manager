#include "src/map_renderer.h"

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "svg/common.h"

#include "src/request_types.h"
#include "src/sphere.h"

const rm::RenderingSettings kTestRenderingSettings{
    .width = 412.1, .height = 395.7, .padding = 170, .stop_radius = 4.2,
    .line_width = 11, .stop_label_font_size = 10,
    .stop_label_offset = svg::Point{.x = -5, .y = 2},
    .underlayer_color = svg::Color(
        svg::Rgba{.red = 1, .green = 100, .blue = 20, .alpha = 0.1}),
    .underlayer_width = 5,
    .color_palette = {
        "purple", svg::Rgb{.red = 15, .green = 23, .blue = 41},
        svg::Rgba{.red = 210, .green = 81, .blue = 14, .alpha = 0.94}}
};

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
const std::pair<std::string_view, rm::sphere::Coords> kDostoevsky =
    {"Dostoevsky street", {.latitude = 13.102921, .longitude = 12.432091}};

TEST(TestMapRenderer, TestInitializing) {
  struct TestCase {
    std::string name;
    std::map<std::string_view, std::vector<std::string_view>> buses;
    std::map<std::string_view, rm::sphere::Coords> stops;
    rm::RenderingSettings settings;
    bool want_fail;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty color palette",
          .settings = [] {
            auto item = kTestRenderingSettings;
            item.color_palette = {};
            return item;
          }(),
          .want_fail = true
      },
      TestCase{
          .name = "Latitude out of range(below)",
          .stops = {{"Airport", {.latitude = -91, .longitude = 0}}},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Latitude out of range(above)",
          .stops = {{"Airport", {.latitude = 91, .longitude = 0}}},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Longitude out of range(below)",
          .stops = {{"Airport", {.latitude = 0, .longitude = -184}}},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Longitude out of range(above)",
          .stops = {{"Airport", {.latitude = 0, .longitude = 197}}},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Bus route has unknown stop",
          .buses = {{"Bus 1", {"Airport", "Dostoevsky", "Clemens"}}},
          .stops = {kAirport, kClemens, kRWStation},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Valid config",
          .buses = {{"Bus 1", {"Airport", "Clemens street", "RW station",
                               "High Street"}}},
          .stops = {kAirport, kClemens, kRWStation, kHighStreet},
          .settings = kTestRenderingSettings,
          .want_fail = false
      }
  };

  for (auto &[name, buses, stops, settings, want_fail] : test_cases) {
    auto want = !want_fail;
    auto got = (rm::MapRenderer::Create(buses, stops, settings) != nullptr);
    EXPECT_EQ(want, got) << name;
  }
}
