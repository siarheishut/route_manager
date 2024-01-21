#include "src/map_renderer.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "svg/common.h"

#include "src/common.h"
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

#define UNDERLAY                                                               \
"<rect x=\"-0\" y=\"-0\" width=\"412.1\" height=\"395.7\" "                    \
"fill=\"rgba(1,100,20,0.1)\" stroke=\"none\" stroke-width=\"1\" />"

#define PREFIX "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"                   \
               "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

#define POSTFIX "</svg>"

#define SVG_DOC(body) PREFIX body POSTFIX

using namespace rm::utils;

const RenderingSettings kTestRenderingSettings{
    .frame = Frame{
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
    .layers = {MapLayer::kBusLines, MapLayer::kBusLabels,
               MapLayer::kStopPoints, MapLayer::kStopLabels}
};

const rm::utils::PostStopRequest kAirport =
    {.stop = "Airport",
        .coords = {.latitude = 12.312245, .longitude = 52.119401}};
const rm::utils::PostStopRequest kShop =
    {.stop = "Shop", .coords = {.latitude = 73.132015, .longitude = -46.132028}};
const rm::utils::PostStopRequest kHighStreet =
    {.stop = "High Street",
        .coords = {.latitude = 42.111111, .longitude = 19.291527}};
const rm::utils::PostStopRequest kRWStation =
    {.stop = "RW station",
        .coords = {.latitude = 63.015232, .longitude = -35.621022}};
const rm::utils::PostStopRequest kClemens =
    {.stop = "Clemens street",
        .coords = {.latitude = 49.102842, .longitude = 39.845290}};

TEST(TestMapRenderer, TestInitializingValid) {
  auto catalog = rm::TransportCatalog::Create(std::vector<PostRequest>{});
  ASSERT_TRUE(catalog) << "Valid config";
  auto got =
      rm::MapRenderer::Create(std::move(catalog), kTestRenderingSettings);
  EXPECT_NE(got, nullptr) << "Valid config";
}

TEST(TestMapRenderer, TestInitializingInvalid) {
  auto catalog = rm::TransportCatalog::Create(std::vector<PostRequest>{});
  ASSERT_TRUE(catalog) << "Invalid config";
  auto got = [&] {
    auto settings = kTestRenderingSettings;
    settings.color_palette = {};
    return rm::MapRenderer::Create(std::move(catalog), settings);
  }();
  EXPECT_EQ(got, nullptr) << "Invalid config";
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
    std::vector<PostRequest> config;
    RenderingSettings rendering_settings;
    std::string want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty layers",
          .config = {
              PostBusRequest{
                  .bus = "1",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop"}},
              kAirport, kShop},
          .rendering_settings = layers_replace_with({}),
          .want = SVG_DOC()
      },
      TestCase{
          .name = "Repetitive layers",
          .config = {kAirport, kShop},
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
          .config = {
              PostBusRequest{
                  .bus = "1",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop", "Airport"}},
              kAirport, kShop},
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
          .name = "Route with different start and end stops",
          .config = {
              PostBusRequest{
                  .bus = "1",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop", "Airport"}},
              kAirport, kShop},
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
          .name = "Empty map",
          .rendering_settings = kTestRenderingSettings,
          .want =  SVG_DOC("")
      },
      TestCase{
          .name = "No routes",
          .config = {kAirport, kShop, kHighStreet},
          .rendering_settings = kTestRenderingSettings,
          .want =  SVG_DOC(
                       STOP(170, 225.7)STOP(170, 225.7)STOP(170, 225.7)
                       STOP_NAME(Airport, 170, 225.7)
                       STOP_NAME(High Street, 170, 225.7)
                       STOP_NAME(Shop, 170, 225.7))
      },
      TestCase{
          .name = "All stops are used",
          .config = {
              PostBusRequest{
                  .bus = "bus 1",
                  .stops = {"Airport", "High Street", "Shop", "Airport"},
                  .endpoints = {"Airport"}},
              PostBusRequest{.bus = "bus 2",
                  .stops = {"High Street", "Airport", "Shop",
                            "RW station", "Shop", "Airport",
                            "High Street"},
                  .endpoints = {"High Street"}},
              kAirport, kShop, kHighStreet, kRWStation},
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
          .config = {
              PostBusRequest{
                  .bus = "Bus 1",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop", "Airport"}},
              PostBusRequest{
                  .bus = "Bus 2",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop"}},
              PostBusRequest{
                  .bus = "Bus 3",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop", "Airport"}},
              PostBusRequest{
                  .bus = "Bus 4",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop"}},
              PostBusRequest{
                  .bus = "Bus 5",
                  .stops = {"Shop", "Airport", "Shop"},
                  .endpoints = {"Shop", "Airport"}},
              kShop, kAirport},
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
          .config = {
              PostStopRequest{
                  .stop = "", .coords = {.latitude = 10, .longitude = 10}}},
          .rendering_settings = kTestRenderingSettings,
          .want = SVG_DOC(STOP(170, 225.7)STOP_NAME(, 170, 225.7))
      },
  };

  for (auto &[name, config, settings, want] : test_cases) {
    auto catalog = rm::TransportCatalog::Create(config);
    EXPECT_TRUE(catalog) << name;
    if (!catalog) continue;

    auto map_renderer = MapRenderer::Create(std::move(catalog), settings);
    EXPECT_TRUE(map_renderer) << name;
    if (!map_renderer) continue;

    std::string got = map_renderer->RenderMap();
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestMapRenderer, TestRenderRoute) {
  using namespace rm;

  const std::vector<utils::PostRequest> kTestConfig = {
      PostBusRequest{
          .bus = "Bus 1",
          .stops = {"a", "b", "c", "d", "a"},
          .endpoints = {"a"}
      },
      PostBusRequest{
          .bus = "Bus 2",
          .stops = {"a", "c", "d", "c", "a"},
          .endpoints = {"a", "d"}
      },
      PostBusRequest{
          .bus = "Bus 3",
          .stops = {"d", "e", "a", "b", "d"},
          .endpoints = {"d"}
      },
      PostStopRequest{"a", {1, 1}}, PostStopRequest{"b", {2, 2}},
      PostStopRequest{"c", {3, 3}}, PostStopRequest{"d", {4, 4}},
      PostStopRequest{"e", {5, 5}}};

  struct TestCase {
    std::string name;
    RenderingSettings settings = kTestRenderingSettings;
    RouteInfo route_info;
    std::optional<std::string> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Bus not found",
          .route_info = {.items = {
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 4", .start_idx = 1,
                  .span_count = 2}}},
          .want = std::nullopt
      },
      TestCase{
          .name = "start_idx < 0",
          .route_info = {.items = {
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = -1,
                  .span_count = 2}}},
          .want = std::nullopt
      },
      TestCase{
          .name = "span_count <= 0",
          .route_info = {.items = {
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = 1,
                  .span_count = 0}}},
          .want = std::nullopt
      },
      TestCase{
          .name = "start_idx > route size",
          .route_info = {.items = {
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = 5,
                  .span_count = 2}}},
          .want = std::nullopt
      },
      TestCase{
          .name = "span_count > route size",
          .route_info = {.items = {
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = 1,
                  .span_count = 5}}},
          .want = std::nullopt
      },
      TestCase{
          .name = "start_idx + span_count > route size",
          .route_info = {.items = {
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = 3,
                  .span_count = 2}}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Common request",
          .route_info = {.items = {
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 1", .start_idx = 1,
                  .span_count = 1},
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = 1,
                  .span_count = 1},
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 3", .start_idx = 0,
                  .span_count = 3},
          }},
          .want = SVG_DOC(
                      ROUTE("purple") POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(170, 225.7)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(242.1, 170)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(242.1, 170)
                      ROUTE_END()
                      BUS_NAME(Bus 1, 170, 225.7, "purple")
                      BUS_NAME(Bus 2, 170, 225.7, "rgb(15,23,41)")
                      BUS_NAME(Bus 2, 242.1, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 3, 242.1, 170, "rgba(210,81,14,0.94)")
                      STOP(170, 225.7) STOP(194.033, 207.133)
                      STOP(218.067, 188.567) STOP(242.1, 170)
                      STOP(194.033, 207.133)
                      STOP_NAME(a, 170, 225.7) STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567) STOP_NAME(d, 242.1, 170)
                      STOP_NAME(e, 194.033, 207.133)
                      UNDERLAY
                      ROUTE("purple") POINT(194.033, 207.133)
                      NEXT_POINT(218.067, 188.567) ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) ROUTE_END()
                      BUS_NAME(Bus 2, 242.1, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 3, 242.1, 170, "rgba(210,81,14,0.94)")
                      STOP(194.033, 207.133) STOP(218.067, 188.567)
                      STOP(218.067, 188.567) STOP(242.1, 170) STOP(242.1, 170)
                      STOP(194.033, 207.133) STOP(170, 225.7)
                      STOP(194.033, 207.133)
                      STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567)
                      STOP_NAME(d, 242.1, 170)
                      STOP_NAME(b, 194.033, 207.133))
      },
      TestCase{
          .name = "Different layers",
          .settings = [&]() {
            auto settings = kTestRenderingSettings;
            settings.layers = {MapLayer::kBusLines, MapLayer::kBusLines,
                               MapLayer::kStopLabels, MapLayer::kBusLabels,
                               MapLayer::kStopLabels};
            return settings;
          }(),
          .route_info = {.items = {
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 1", .start_idx = 1,
                  .span_count = 1},
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 2", .start_idx = 1,
                  .span_count = 1},
              RouteInfo::WaitItem{},
              RouteInfo::RoadItem{.bus = "Bus 3", .start_idx = 0,
                  .span_count = 3},
          }},
          .want = SVG_DOC(
                      ROUTE("purple") POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(170, 225.7)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(242.1, 170)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(242.1, 170)
                      ROUTE_END()
                      ROUTE("purple") POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(170, 225.7)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(242.1, 170)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(242.1, 170)
                      ROUTE_END()
                      STOP_NAME(a, 170, 225.7) STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567) STOP_NAME(d, 242.1, 170)
                      STOP_NAME(e, 194.033, 207.133)
                      BUS_NAME(Bus 1, 170, 225.7, "purple")
                      BUS_NAME(Bus 2, 170, 225.7, "rgb(15,23,41)")
                      BUS_NAME(Bus 2, 242.1, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 3, 242.1, 170, "rgba(210,81,14,0.94)")
                      STOP_NAME(a, 170, 225.7) STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567) STOP_NAME(d, 242.1, 170)
                      STOP_NAME(e, 194.033, 207.133)
                      UNDERLAY
                      ROUTE("purple") POINT(194.033, 207.133)
                      NEXT_POINT(218.067, 188.567) ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) ROUTE_END()
                      ROUTE("purple") POINT(194.033, 207.133)
                      NEXT_POINT(218.067, 188.567) ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) ROUTE_END()
                      STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567)
                      STOP_NAME(d, 242.1, 170)
                      STOP_NAME(b, 194.033, 207.133)
                      BUS_NAME(Bus 2, 242.1, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 3, 242.1, 170, "rgba(210,81,14,0.94)")
                      STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567)
                      STOP_NAME(d, 242.1, 170)
                      STOP_NAME(b, 194.033, 207.133))
      },
      TestCase{
          .name = "WaitItem",
          .route_info = {.items = {
              RouteInfo::WaitItem{},
              RouteInfo::WaitItem{},
              RouteInfo::WaitItem{},
              RouteInfo::WaitItem{},
          }},
          .want = SVG_DOC(
                      ROUTE("purple") POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(218.067, 188.567)
                      NEXT_POINT(242.1, 170) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgb(15,23,41)") POINT(170, 225.7)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(242.1, 170)
                      NEXT_POINT(218.067, 188.567) NEXT_POINT(170, 225.7)
                      ROUTE_END()
                      ROUTE("rgba(210,81,14,0.94)") POINT(242.1, 170)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(170, 225.7)
                      NEXT_POINT(194.033, 207.133) NEXT_POINT(242.1, 170)
                      ROUTE_END()
                      BUS_NAME(Bus 1, 170, 225.7, "purple")
                      BUS_NAME(Bus 2, 170, 225.7, "rgb(15,23,41)")
                      BUS_NAME(Bus 2, 242.1, 170, "rgb(15,23,41)")
                      BUS_NAME(Bus 3, 242.1, 170, "rgba(210,81,14,0.94)")
                      STOP(170, 225.7) STOP(194.033, 207.133)
                      STOP(218.067, 188.567) STOP(242.1, 170)
                      STOP(194.033, 207.133)
                      STOP_NAME(a, 170, 225.7) STOP_NAME(b, 194.033, 207.133)
                      STOP_NAME(c, 218.067, 188.567) STOP_NAME(d, 242.1, 170)
                      STOP_NAME(e, 194.033, 207.133)
                      UNDERLAY)
      },
  };

  for (auto &[name, settings, route_info, want] : test_cases) {
    auto catalog = rm::TransportCatalog::Create(kTestConfig);
    EXPECT_TRUE(catalog) << name;
    if (!catalog) continue;

    auto map_renderer = rm::MapRenderer::Create(std::move(catalog), settings);
    EXPECT_TRUE(map_renderer != nullptr) << name << ": factory method failed";
    if (!map_renderer) continue;
    auto got = map_renderer->RenderRoute(route_info);
    EXPECT_EQ(want, got) << name;
  }
}
