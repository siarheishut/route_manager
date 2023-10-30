#include <optional>
#include <string>
#include <vector>

#include "json.h"
#include "gtest/gtest.h"

#include "src/request_parser.h"
#include "src/request_types.h"
#include "tests/test_utils.h"

TEST(TestParseSettings, TestRoutingSettings) {
  using namespace rm;

  struct TestCase {
    std::string name;
    json::Dict input;
    std::optional<RoutingSettings> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong request: no <bus_wait_time>",
          .input = json::Dict{{"bus_velocity", 36.4},
                              {"bus_wait", 10}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: no <bus_velocity>",
          .input = json::Dict{{"bus_speed", 36.4},
                              {"bus_wait_time", 10}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: <bus_wait_time> isn't int",
          .input = json::Dict{{"bus_velocity", 42.2},
                              {"bus_wait_time", 4.7}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong request: <bus_velocity> isn't double",
          .input = json::Dict{{"bus_velocity", "41.3"},
                              {"bus_wait_time", 6}},
          .want = std::nullopt
      },
      TestCase{
          .name = "Common settings",
          .input = json::Dict{{"bus_velocity", 10.1},
                              {"bus_wait_time", 10}},
          .want = RoutingSettings{.bus_wait_time = 10, .bus_velocity = 10.1}
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseRoutingSettings(input);
    EXPECT_EQ(want, got);
  }
}

TEST(TestParseSettings, TestRedneringSettings) {
  using namespace std;
  using namespace rm;

  struct TestCase {
    string name;
    json::Dict input;
    optional<RenderingSettings> want;
  };

  const auto test_item = json::Dict{{"width", 412.1},
                                    {"height", 395.7},
                                    {"padding", 210},
                                    {"stop_radius", 4.2},
                                    {"line_width", 11},
                                    {"stop_label_font_size", 10},
                                    {"stop_label_offset",
                                     json::List{-5, 2}},
                                    {"underlayer_color",
                                     json::List{1, 100, 20, 0.1}},
                                    {"underlayer_width", 5},
                                    {"color_palette", json::List{
                                        "purple", json::List{15, 23, 41},
                                        json::List{210, 81, 14, 0.94}}},
                                    {"bus_label_font_size", 6},
                                    {"bus_label_offset", json::List{5, -7}},
                                    {"layers", json::List{"bus_labels",
                                                          "stop_points",
                                                          "bus_lines",
                                                          "stop_labels"}},
                                    {"outer_margin", 20.34}};
  auto test_item_without = [&](string field) {
    auto item = test_item;
    item.erase(field);
    return item;
  };
  auto test_item_replace = [&](string field, json::Node node) {
    auto item = test_item;
    item[field] = std::move(node);
    return item;
  };

  const auto settings = RenderingSettings{
      .frame = renderer_utils::Frame{
          .width = 412.1, .height = 395.7, .padding = 210,
      },
      .stop_radius = 4.2, .line_width = 11, .stop_label_font_size = 10,
      .stop_label_offset = svg::Point{.x = -5, .y = 2},
      .underlayer_color = svg::Color(
          svg::Rgba{.red = 1, .green = 100, .blue = 20, .alpha = 0.1}),
      .underlayer_width = 5,
      .color_palette = {
          "purple", svg::Rgb{.red = 15, .green = 23, .blue = 41},
          svg::Rgba{.red = 210, .green = 81, .blue = 14, .alpha = 0.94}},
      .bus_label_font_size = 6,
      .bus_label_offset = {.x = 5, .y = -7},
      .layers = {MapLayer::kBusLabels,
                 MapLayer::kStopPoints,
                 MapLayer::kBusLines,
                 MapLayer::kStopLabels},
      .outer_margin = 20.34,
  };

  auto settings_replace_layers_with = [&](vector<MapLayer> layers) {
    auto item = settings;
    item.layers = std::move(layers);
    return item;
  };

  vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong format: no <width>",
          .input = test_item_without("width"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <height>",
          .input = test_item_without("height"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <padding>",
          .input = test_item_without("padding"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <stop_radius>",
          .input = test_item_without("stop_radius"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <line_width>",
          .input = test_item_without("line_width"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <stop_label_font_size>",
          .input = test_item_without("stop_label_font_size"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <stop_label_offset>",
          .input = test_item_without("stop_label_offset"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <underlayer_color>",
          .input = test_item_without("underlayer_color"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <underlayer_width>",
          .input = test_item_without("underlayer_width"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <color_palette>",
          .input = test_item_without("color_palette"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <bus_label_font_size>",
          .input = test_item_without("bus_label_font_size"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <bus_label_offset>",
          .input = test_item_without("bus_label_offset"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <layers>",
          .input = test_item_without("layers"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: no <outer_margin>",
          .input = test_item_without("outer_margin"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <width> isn't double",
          .input = test_item_replace("width", "123.4"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <height> isn't double",
          .input = test_item_replace("height", "321"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <padding> isn't double",
          .input = test_item_replace("padding", "214.7"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <stop_radius> isn't double",
          .input = test_item_replace("stop_radius", "8152"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <line_width> isn't double",
          .input = test_item_replace("line_width", "1234"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <stop_label_font_size> isn't int",
          .input = test_item_replace("stop_label_font_size", "123.4"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <stop_label_offset> isn't array",
          .input = test_item_replace("stop_label_offset",
                                     json::Dict{{"key", "value"}}),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <underlayer_color> isn't color",
          .input = test_item_replace("underlayer_color", 381),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <underlayer_width> isn't double",
          .input = test_item_replace("underlayer_width", "937.5"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <color_palette> isn't array",
          .input = test_item_replace("color_palette", "color1"),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <underlayer_color> isn't color",
          .input = test_item_replace("underlayer_color", 185),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <color_palette> contains not color",
          .input = test_item_replace("color_palette",
                                     json::List{"red", "purple", 15}),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <bus_label_font_size> isn't int",
          .input = test_item_replace("bus_label_font_size", 12.3),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <bus_label_offset> isn't array",
          .input = test_item_replace("bus_label_offset", 14.5),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <layers> isn't array",
          .input = test_item_replace("layers", json::Dict{{"1", 1}, {"2", 2}}),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <layers> contains not string",
          .input = test_item_replace("layers", json::List{"1", "2", 3}),
          .want = nullopt
      },
      TestCase{
          .name = "Wrong format: <outer_margin> isn't double",
          .input = test_item_replace("outer_margin", "42.1"),
          .want = nullopt
      },
      TestCase{
          .name = "Empty layers",
          .input = test_item_replace("layers", json::List{}),
          .want = settings_replace_layers_with({})
      },
      TestCase{
          .name = "Invalid layer strings",
          .input = test_item_replace(
              "layers", json::List{"1", "2", "3", "2", "bus_labels"}),
          .want = settings_replace_layers_with({MapLayer::kBusLabels})
      },
      TestCase{
          .name = "Repeated layers",
          .input = test_item_replace(
              "layers", json::List{"bus_labels", "stop_labels",
                                   "bus_labels", "stop_labels"}),
          .want = settings_replace_layers_with(
              {MapLayer::kBusLabels, MapLayer::kStopLabels,
               MapLayer::kBusLabels, MapLayer::kStopLabels})
      },
      TestCase{
          .name = "Valid settings",
          .input = test_item,
          .want = settings
      }
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = ParseRenderingSettings(input);
    EXPECT_EQ(want, got) << name;
  }
}
