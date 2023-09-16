#include "color_parser.h"

#include <algorithm>

#include "json.h"
#include "svg/common.h"

namespace rm {
bool IsColor(const json::Node &node) {
  if (node.IsString()) return true;

  if (!node.IsArray()) return false;
  auto &arr = node.AsArray();

  bool rgb = arr.size() == 3;
  bool rgba = arr.size() == 4;
  if (!rgb & !rgba) return false;

  bool colors = std::all_of(arr.begin(), arr.begin() + 3, [](auto &item) {
    return item.IsInt();
  });

  if (rgb) return colors;

  bool alpha = arr[3].IsDouble();
  return colors && alpha;
}

svg::Color AsColor(json::Node node) {
  if (node.IsString()) {
    return svg::Color{node.ReleaseString()};
  }
  auto color = node.ReleaseArray();
  if (color.size() == 3) {
    return svg::Color{svg::Rgb{
        .red = static_cast<uint8_t>(color[0].AsInt()),
        .green = static_cast<uint8_t>(color[1].AsInt()),
        .blue = static_cast<uint8_t>(color[2].AsInt()),
    }};
  }
  return svg::Color{svg::Rgba{
      .red = static_cast<uint8_t>(color[0].AsInt()),
      .green = static_cast<uint8_t>(color[1].AsInt()),
      .blue = static_cast<uint8_t>(color[2].AsInt()),
      .alpha = color[3].AsDouble()
  }};
}
}
