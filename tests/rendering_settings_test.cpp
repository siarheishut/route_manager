#include "src/request_types.h"

#include <variant>

#include "gtest/gtest.h"
#include "svg/common.h"

#include "rendering_settings.pb.h"
#include "test_utils.h"

TEST(TestRenderingSettings, TestSerializationAndDeserialization) {
  auto initial = rm::utils::RenderingSettings{
      .frame = rm::utils::Frame{.width = 412.1, .height = 395.7, .padding = 210},
      .stop_radius = 4.2, .line_width = 11, .stop_label_font_size = 10,
      .stop_label_offset = svg::Point{.x = -5, .y = 2},
      .underlayer_color = "red",
      .underlayer_width = 5,
      .color_palette = {
          svg::Rgba{.red = 1, .green = 100, .blue = 20, .alpha = 0.1},
          "purple",
          svg::Rgb{.red = 15, .green = 23, .blue = 41},
          svg::kNoneColor},
      .bus_label_font_size = 6,
      .bus_label_offset = {.x = 5, .y = -7},
      .layers = {rm::utils::MapLayer::kBusLabels,
                 rm::utils::MapLayer::kStopLabels,
                 rm::utils::MapLayer::kStopPoints,
                 rm::utils::MapLayer::kStopPoints,
                 rm::utils::MapLayer::kBusLines,
                 rm::utils::MapLayer::kBusLabels,
                 rm::utils::MapLayer::kStopLabels},
      .outer_margin = 20.34,
  };

  auto serialized = initial.Serialize();
  rm::utils::RenderingSettings deserialized;
  deserialized.Deserialize(serialized);
  ASSERT_EQ(initial, deserialized) << "Deserialization failed";
  deserialized.Deserialize(serialized);
  ASSERT_EQ(initial, deserialized) << "Repeated deserialization failed";
}
