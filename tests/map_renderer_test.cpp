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

#define STOP(x, y)                                                             \
 "<circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" cx=\""#x"\" "      \
 "cy=\""#y"\" r=\"4.2\"/>"

#define ROUTE(color)                                                           \
 "<polyline fill=\"none\" stroke=\"" color "\" stroke-width=\"11\" "           \
 "stroke-linecap=\"round\" stroke-linejoin=\"round\" points=\""

#define POINT(x, y) #x "," #y

#define NEXT_POINT(x, y) " " #x "," #y

#define ROUTE_END() "\"/>"

#define TEXT(stop, x, y)                                                       \
 "<text fill=\"rgba(1,100,20,0.1)\" stroke=\"rgba(1,100,"                      \
 "20,0.1)\" stroke-width=\"5\" stroke-linecap=\"round\" "                      \
 "stroke-linejoin=\"round\" x=\"" #x "\" y=\"" #y "\" dx=\"-5\" dy=\"2\" "     \
 "font-size=\"10\" font-family=\"Verdana\">" #stop "</text>"                   \
 "<text fill=\"black\" stroke=\"none\" stroke-width=\"1\" "                    \
 "x=\""#x"\" y=\"" #y "\" dx=\"-5\" dy=\"2\" font-size=\"10\" font-family=\""  \
 "Verdana\">" #stop "</text>"

#define PREFIX "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"                   \
               "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

#define POSTFIX "</svg>"

#define SVG_DOC(body) PREFIX body POSTFIX

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

TEST(TestMapRenderer, TestGetMap) {
  using namespace rm;

  struct TestCase {
    std::string name;
    std::map<std::string_view, std::vector<std::string_view>> buses;
    std::map<std::string_view, sphere::Coords> stops;
    RenderingSettings rendering_settings;
    std::string want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty map",
          .rendering_settings = kTestRenderingSettings,
          .want =  SVG_DOC("")
      },
      TestCase{
          .name = "No routes",
          .stops = {kAirport, kShop, kHighStreet},
          .rendering_settings = kTestRenderingSettings,
          .want =  SVG_DOC(
                       STOP(242.1, 214.631)STOP(218.01, 192.764)STOP(170, 170)
                       TEXT(Airport, 242.1, 214.631)
                       TEXT(High Street, 218.01, 192.764)
                       TEXT(Shop, 170, 170))
      },
      TestCase{
          .name = "All stops are used",
          .buses = {{"bus 1", {"Airport", "High Street", "Shop", "Airport"}},
                    {"bus 2", {"High Street", "Airport", "Shop",
                               "RW station", "Shop", "Airport",
                               "High Street"}}},
          .stops = {kAirport, kShop, kHighStreet, kRWStation},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")
                      POINT(242.1, 214.631)NEXT_POINT(218.01, 192.764)
                      NEXT_POINT(170, 170)NEXT_POINT(242.1, 214.631)
                      ROUTE_END()
                      ROUTE("rgb(15,23,41)")
                      POINT(218.01, 192.764)
                      NEXT_POINT(242.1, 214.631) NEXT_POINT(170, 170)
                      NEXT_POINT(177.713, 177.424) NEXT_POINT(170, 170)
                      NEXT_POINT(242.1, 214.631) NEXT_POINT(218.01, 192.764)
                      ROUTE_END()
                      STOP(242.1, 214.631) STOP(218.01, 192.764)
                      STOP(177.713, 177.424) STOP(170, 170)
                      TEXT(Airport, 242.1, 214.631)
                      TEXT(High Street, 218.01, 192.764)
                      TEXT(RW station, 177.713, 177.424)
                      TEXT(Shop, 170, 170))
      },
      TestCase{
          .name = "Route number is greater than palette size",
          .buses = {{"Bus 1", {"Shop"}}, {"Bus 2", {"Shop"}},
                    {"Bus 3", {"Shop"}}, {"Bus 4", {"Shop"}},
                    {"Bus 5", {"Shop"}}},
          .stops = {kShop},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")POINT(170, 170)ROUTE_END()
                      ROUTE("rgb(15,23,41)")POINT(170, 170)ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)")POINT(170, 170)ROUTE_END()
                      ROUTE("purple")POINT(170, 170)ROUTE_END()
                      ROUTE("rgb(15,23,41)")POINT(170, 170)ROUTE_END()
                      STOP(170, 170)TEXT(Shop, 170, 170))
      },
      TestCase{
          .name = "Empty stop name",
          .stops = {{"", {.latitude = 10, .longitude = 10}}},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(STOP(170, 170)TEXT(, 170, 170))
      },
      TestCase{
          .name = "Several stops with the same coords",
          .stops = {{"RW", {.latitude = 10, .longitude = 10}},
                    {"Stop", {.latitude = 10, .longitude = 10}}},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      STOP(170, 170)STOP(170, 170)
                      TEXT(RW, 170, 170)TEXT(Stop, 170, 170))
      },
  };

  for (auto &[name, buses, stops, settings, want] : test_cases) {
    std::string got = MapRenderer::Create(buses, stops, settings)->GetMap();
    EXPECT_EQ(want, got) << name;
  }
}