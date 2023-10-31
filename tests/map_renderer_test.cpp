#include "src/map_renderer.h"

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "svg/common.h"

#include "src/map_renderer_utils.h"
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

#define BUS_NAME(bus, x, y, color)                                             \
 "<text fill=\"rgba(1,100,20,0.1)\" stroke=\"rgba(1,100,20,0.1)\" "            \
 "stroke-width=\"5\" stroke-linecap=\"round\" stroke-linejoin=\"round\" "      \
 "x=\"" #x "\" y=\"" #y "\" dx=\"5\" dy=\"-7\" font-size=\"6\" "               \
 "font-family=\"Verdana\" font-weight=\"bold\">" #bus "</text><text fill=\""   \
 color "\" stroke=\"none\" stroke-width=\"1\" x=\""#x"\" y=\"" #y "\" "        \
 "dx=\"5\" dy=\"-7\" font-size=\"6\" font-family=\"Verdana\" "                 \
 "font-weight=\"bold\">" #bus "</text>"

#define STOP_NAME(stop, x, y)                                                  \
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
    .frame = rm::renderer_utils::Frame{
        .width = 412.1, .height = 395.7, .padding = 170,
    },
    .stop_radius = 4.2, .line_width = 11,
    .stop_label_font_size = 10,
    .stop_label_offset = svg::Point{.x = -5, .y = 2},
    .underlayer_color = svg::Color(
        svg::Rgba{.red = 1, .green = 100, .blue = 20, .alpha = 0.1}),
    .underlayer_width = 5,
    .color_palette = {
        "purple", svg::Rgb{.red = 15, .green = 23, .blue = 41},
        svg::Rgba{.red = 210, .green = 81, .blue = 14, .alpha = 0.94}},
    .bus_label_font_size = 6,
    .bus_label_offset = svg::Point{.x = 5, .y = -7},
    .layers = {rm::MapLayer::kBusLines, rm::MapLayer::kBusLabels,
               rm::MapLayer::kStopPoints, rm::MapLayer::kStopLabels}
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

TEST(TestMapRenderer, TestInitializing) {
  struct TestCase {
    std::string name;
    std::map<std::string_view, rm::renderer_utils::Route> buses;
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
          .buses = {{"Bus 1", {{"Airport", "Dostoevsky", "Clemens"}, false}}},
          .stops = {kAirport, kClemens, kRWStation},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Bus route has unknown stop",
          .buses = {{"Bus 1", {{"Airport", "Dostoevsky", "Clemens",
                                "Airport"}, true}}},
          .stops = {kAirport, kClemens, kRWStation},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Roundtrip route has different end points",
          .buses = {{"Bus 1", {{"Airport", "RW station", "Clemens"}, true}}},
          .stops = {kAirport, kClemens, kRWStation},
          .settings = kTestRenderingSettings,
          .want_fail = true
      },
      TestCase{
          .name = "Valid config",
          .buses = {{"Bus 1", {{"Airport", "Clemens street", "RW station",
                                "High Street", "Airport"}, true}}},
          .stops = {kAirport, kClemens, kRWStation, kHighStreet},
          .settings = kTestRenderingSettings,
          .want_fail = false
      },
      TestCase{
          .name = "Valid config",
          .buses = {{"Bus 1", {{"Airport", "RW station"}, false}}},
          .stops = {kAirport, kRWStation, kClemens},
          .settings = kTestRenderingSettings,
          .want_fail = false
      },
  };

  for (auto &[name, buses, stops, settings, want_fail] : test_cases) {
    auto want = !want_fail;
    auto got = (rm::MapRenderer::Create(buses, stops, settings) != nullptr);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestMapRenderer, TestRenderMap) {
  using namespace rm;

  auto layers_replace_with = [&](std::vector<MapLayer> layers) {
    auto item = kTestRenderingSettings;
    item.layers = std::move(layers);
    return item;
  };

  struct TestCase {
    std::string name;
    std::map<std::string_view, rm::renderer_utils::Route> buses;
    std::map<std::string_view, sphere::Coords> stops;
    RenderingSettings rendering_settings;
    std::string want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty layers",
          .buses = {{"1", {{"Shop", "Airport"}, false}}},
          .stops = {kAirport, kShop},
          .rendering_settings = layers_replace_with({}),
          .want = SVG_DOC()
      },
      TestCase{
          .name = "Repetitive layers",
          .stops = {kAirport, kShop},
          .rendering_settings = layers_replace_with({MapLayer::kStopLabels,
                                                     MapLayer::kStopLabels,
                                                     MapLayer::kStopPoints,
                                                     MapLayer::kStopPoints}),
          .want = SVG_DOC(
                      STOP_NAME(Airport, 170, 225.7)
                      STOP_NAME(Shop, 170, 225.7)
                      STOP_NAME(Airport, 170, 225.7)
                      STOP_NAME(Shop, 170, 225.7)
                      STOP(170, 225.7)STOP(170, 225.7)
                      STOP(170, 225.7)STOP(170, 225.7))
      },
      TestCase{
          .name = "Random order",
          .buses = {{"1", {{"Shop", "Airport"}, false}}},
          .stops = {kAirport, kShop},
          .rendering_settings =  layers_replace_with({MapLayer::kStopLabels,
                                                      MapLayer::kBusLines,
                                                      MapLayer::kStopPoints,
                                                      MapLayer::kBusLabels,
                                                      MapLayer::kStopLabels}),
          .want = SVG_DOC(
                      STOP_NAME(Airport, 242.1, 225.7)
                      STOP_NAME(Shop, 170, 170)
                      ROUTE("purple")
                      POINT(170, 170)NEXT_POINT(242.1, 225.7)
                      NEXT_POINT(170, 170) ROUTE_END()
                      STOP(242.1, 225.7)STOP(170, 170)
                      BUS_NAME(1, 170, 170, "purple")
                      BUS_NAME(1, 242.1, 225.7, "purple")
                      STOP_NAME(Airport, 242.1, 225.7)
                      STOP_NAME(Shop, 170, 170))
      },
      TestCase{
          .name = "Roundtrip",
          .buses = {{"1", {{"Shop", "Airport", "Shop"}, true}}},
          .stops = {kAirport, kShop},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")
                      POINT(242.1, 170)NEXT_POINT(170, 225.7)
                      NEXT_POINT(242.1, 170) ROUTE_END()
                      BUS_NAME(1, 242.1, 170, "purple")
                      STOP(170, 225.7)STOP(242.1, 170)
                      STOP_NAME(Airport, 170, 225.7)
                      STOP_NAME(Shop, 242.1, 170))
      },
      TestCase{
          .name = "Non-roundrip with difference start and end stops",
          .buses = {{"1", {{"Shop", "Airport"}, false}}},
          .stops = {kAirport, kShop},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")
                      POINT(170, 170)NEXT_POINT(242.1, 225.7)
                      NEXT_POINT(170, 170) ROUTE_END()
                      BUS_NAME(1, 170, 170, "purple")
                      BUS_NAME(1, 242.1, 225.7, "purple")
                      STOP(242.1, 225.7)STOP(170, 170)
                      STOP_NAME(Airport, 242.1, 225.7)
                      STOP_NAME(Shop, 170, 170))
      },
      TestCase{
          .name = "Non-roundrip with the same start and end stops",
          .buses = {{"1", {{"Shop", "Airport", "Shop"}, false}}},
          .stops = {kAirport, kShop},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")
                      POINT(242.1, 170)NEXT_POINT(170, 225.7)
                      NEXT_POINT(242.1, 170)NEXT_POINT(170, 225.7)
                      NEXT_POINT(242.1, 170)ROUTE_END()
                      BUS_NAME(1, 242.1, 170, "purple")
                      STOP(170, 225.7)STOP(242.1, 170)
                      STOP_NAME(Airport, 170, 225.7)
                      STOP_NAME(Shop, 242.1, 170))
      },
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
                       STOP(170, 225.7)STOP(170, 225.7)STOP(170, 225.7)
                       STOP_NAME(Airport, 170, 225.7)
                       STOP_NAME(High Street, 170, 225.7)
                       STOP_NAME(Shop, 170, 225.7))
      },
      TestCase{
          .name = "All stops are used",
          .buses = {
              {"bus 1", {{"Airport", "High Street", "Shop", "Airport"}, true}},
              {"bus 2", {{"High Street", "Airport", "Shop",
                          "RW station", "Shop", "Airport",
                          "High Street"}, true}}},
          .stops = {kAirport, kShop, kHighStreet, kRWStation},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")
                      POINT(242.1, 225.7)NEXT_POINT(218.067, 197.85)
                      NEXT_POINT(194.033, 170)NEXT_POINT(242.1, 225.7)
                      ROUTE_END()
                      ROUTE("rgb(15,23,41)")
                      POINT(218.067, 197.85)
                      NEXT_POINT(242.1, 225.7) NEXT_POINT(194.033, 170)
                      NEXT_POINT(170, 225.7) NEXT_POINT(194.033, 170)
                      NEXT_POINT(242.1, 225.7) NEXT_POINT(218.067, 197.85)
                      ROUTE_END()
                      BUS_NAME(bus 1, 242.1, 225.7, "purple")
                      BUS_NAME(bus 2, 218.067, 197.85, "rgb(15,23,41)")
                      STOP(242.1, 225.7) STOP(218.067, 197.85)
                      STOP(170, 225.7) STOP(194.033, 170)
                      STOP_NAME(Airport, 242.1, 225.7)
                      STOP_NAME(High Street, 218.067, 197.85)
                      STOP_NAME(RW station, 170, 225.7)
                      STOP_NAME(Shop, 194.033, 170))
      },
      TestCase{
          .name = "Route number is greater than palette size",
          .buses = {{"Bus 1", {{"Shop", "Airport"}, false}},
                    {"Bus 2", {{"Shop", "Airport", "Shop"}, true}},
                    {"Bus 3", {{"Shop", "Airport"}, false}},
                    {"Bus 4", {{"Shop", "Airport", "Shop"}, true}},
                    {"Bus 5", {{"Shop", "Airport"}, false}}},
          .stops = {kShop, kAirport},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(
                      ROUTE("purple")POINT(170, 170)NEXT_POINT(242.1, 225.7)
                      NEXT_POINT(170, 170)ROUTE_END()
                      ROUTE("rgb(15,23,41)")POINT(170, 170)
                      NEXT_POINT(242.1, 225.7)NEXT_POINT(170, 170)ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)")POINT(170, 170)
                      NEXT_POINT(242.1, 225.7) NEXT_POINT(170, 170)ROUTE_END()
                      ROUTE("purple")POINT(170, 170)NEXT_POINT(242.1, 225.7)
                      NEXT_POINT(170, 170)ROUTE_END()
                      ROUTE("rgb(15,23,41)")POINT(170, 170)
                      NEXT_POINT(242.1, 225.7)NEXT_POINT(170, 170) ROUTE_END()
                      BUS_NAME(Bus 1, 170, 170, "purple")
                      BUS_NAME(Bus 1, 242.1, 225.7, "purple")
                      BUS_NAME(Bus 2, 170, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 3, 170, 170, "rgba(210,81,14,0.94)")
                      BUS_NAME(Bus 3, 242.1, 225.7, "rgba(210,81,14,0.94)")
                      BUS_NAME(Bus 4, 170, 170, "purple")
                      BUS_NAME(Bus 5, 170, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 5, 242.1, 225.7, "rgb(15,23,41)")
                      STOP(242.1, 225.7)STOP(170, 170)
                      STOP_NAME(Airport, 242.1, 225.7)
                      STOP_NAME(Shop, 170, 170))
      },
      TestCase{
          .name = "Empty stop name",
          .stops = {{"", {.latitude = 10, .longitude = 10}}},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(STOP(170, 225.7)STOP_NAME(, 170, 225.7))
      },
  };

  for (auto &[name, buses, stops, settings, want] : test_cases) {
    std::string got = MapRenderer::Create(buses, stops, settings)->RenderMap();
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestGetSection, TestGetSection) {
  rm::renderer_utils::Buses buses = {
      {"bus 1", {{"Airport", "High Street", "Shop", "Airport"}, true}},
      {"bus 2", {{"High Street", "Airport", "Shop",
                  "RW station", "Shop", "Airport",
                  "High Street"}, true}}};
  rm::renderer_utils::Stops stops = {kAirport, kShop, kHighStreet, kRWStation};
  auto settings = kTestRenderingSettings;

  std::ostringstream ss;
  rm::MapRenderer::Create(buses, stops, settings)->GetSection().Render(ss);
  auto got = ss.str();
  auto want = ROUTE("purple")
              POINT(242.1, 225.7)NEXT_POINT(218.067, 197.85)
              NEXT_POINT(194.033, 170)NEXT_POINT(242.1, 225.7)
              ROUTE_END()
              ROUTE("rgb(15,23,41)")
              POINT(218.067, 197.85)
              NEXT_POINT(242.1, 225.7) NEXT_POINT(194.033, 170)
              NEXT_POINT(170, 225.7) NEXT_POINT(194.033, 170)
              NEXT_POINT(242.1, 225.7) NEXT_POINT(218.067, 197.85)
              ROUTE_END()
              BUS_NAME(bus 1, 242.1, 225.7, "purple")
              BUS_NAME(bus 2, 218.067, 197.85, "rgb(15,23,41)")
              STOP(242.1, 225.7) STOP(218.067, 197.85)
              STOP(170, 225.7) STOP(194.033, 170)
              STOP_NAME(Airport, 242.1, 225.7)
              STOP_NAME(High Street, 218.067, 197.85)
              STOP_NAME(RW station, 170, 225.7)
              STOP_NAME(Shop, 194.033, 170);
  EXPECT_EQ(want, got) << "Base request";
}
