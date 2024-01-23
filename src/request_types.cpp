#include "request_types.h"

#include "svg/common.h"

#include "rendering_settings.pb.h"
#include <variant>

namespace {
::TransportCatalog::Color Serialize(const svg::Color &color) {
  ::TransportCatalog::Color proto_color;
  if (std::holds_alternative<svg::Rgba>(color)) {
    auto &rgba = std::get<svg::Rgba>(color);
    proto_color.mutable_rgba()->set_red(rgba.red);
    proto_color.mutable_rgba()->set_green(rgba.green);
    proto_color.mutable_rgba()->set_blue(rgba.blue);
    proto_color.mutable_rgba()->set_alpha(rgba.alpha);
  } else if (std::holds_alternative<svg::Rgb>(color)) {
    auto &rgb = std::get<svg::Rgb>(color);
    proto_color.mutable_rgb()->set_red(rgb.red);
    proto_color.mutable_rgb()->set_green(rgb.green);
    proto_color.mutable_rgb()->set_blue(rgb.blue);
  } else if (std::holds_alternative<std::string>(color)) {
    proto_color.set_name(std::get<std::string>(color));
  } else {
    proto_color.clear_color();
  }
  return proto_color;
}

svg::Color Deserialize(const ::TransportCatalog::Color &proto_color) {
  if (proto_color.color_case() == ::TransportCatalog::Color::COLOR_NOT_SET) {
    return svg::Color{};
  } else if (proto_color.color_case() == ::TransportCatalog::Color::kName) {
    return svg::Color(proto_color.name());
  } else if (proto_color.color_case() == ::TransportCatalog::Color::kRgba) {
    return svg::Color(
        svg::Rgba{static_cast<uint8_t>(proto_color.rgba().red()),
                  static_cast<uint8_t>(proto_color.rgba().green()),
                  static_cast<uint8_t>(proto_color.rgba().blue()),
                  proto_color.rgba().alpha()});
  } else {
    return svg::Color(
        svg::Rgb{static_cast<uint8_t>(proto_color.rgb().red()),
                 static_cast<uint8_t>(proto_color.rgb().green()),
                 static_cast<uint8_t>(proto_color.rgb().blue())});
  }
}
}

namespace rm {
void utils::RenderingSettings::Deserialize(
    const ::TransportCatalog::RenderingSettings &proto_settings) {
  color_palette.clear();
  layers.clear();

  frame = utils::Frame{proto_settings.frame().width(),
                       proto_settings.frame().height(),
                       proto_settings.frame().padding()};
  stop_radius = proto_settings.stop_radius();
  line_width = proto_settings.line_width();
  stop_label_font_size = proto_settings.stop_label_font_size();
  stop_label_offset = {proto_settings.stop_label_offset().x(),
                       proto_settings.stop_label_offset().y()};
  underlayer_color = ::Deserialize(proto_settings.underlayer_color());
  underlayer_width = proto_settings.underlayer_width();
  for (auto &proto_color : proto_settings.color_palette()) {
    color_palette.push_back(::Deserialize(proto_color));
  }
  bus_label_font_size = proto_settings.bus_label_font_size();
  bus_label_offset = {proto_settings.bus_label_offset().x(),
                      proto_settings.bus_label_offset().y()};
  for (auto proto_layer : proto_settings.layers()) {
    if (proto_layer == ::TransportCatalog::MapLayer::kBusLines) {
      layers.push_back(utils::MapLayer::kBusLines);
    } else if (proto_layer == ::TransportCatalog::MapLayer::kBusLabels) {
      layers.push_back(utils::MapLayer::kBusLabels);
    } else if (proto_layer == ::TransportCatalog::MapLayer::kStopPoints) {
      layers.push_back(utils::MapLayer::kStopPoints);
    } else {
      layers.push_back(utils::MapLayer::kStopLabels);
    }
  }
  outer_margin = proto_settings.outer_margin();
}

::TransportCatalog::RenderingSettings utils::RenderingSettings::Serialize() const {
  ::TransportCatalog::RenderingSettings proto_settings;
  proto_settings.mutable_frame()->set_width(frame.width);
  proto_settings.mutable_frame()->set_height(frame.height);
  proto_settings.mutable_frame()->set_padding(frame.padding);
  proto_settings.set_stop_radius(stop_radius);
  proto_settings.set_line_width(line_width);
  proto_settings.set_stop_label_font_size(stop_label_font_size);
  proto_settings.mutable_stop_label_offset()->set_x(stop_label_offset.x);
  proto_settings.mutable_stop_label_offset()->set_y(stop_label_offset.y);
  *proto_settings.mutable_underlayer_color() = ::Serialize(underlayer_color);
  proto_settings.set_underlayer_width(underlayer_width);
  for (auto &color : color_palette) {
    *proto_settings.add_color_palette() = ::Serialize(color);
  }
  proto_settings.set_bus_label_font_size(bus_label_font_size);
  proto_settings.mutable_bus_label_offset()->set_x(bus_label_offset.x);
  proto_settings.mutable_bus_label_offset()->set_y(bus_label_offset.y);
  for (auto layer : layers) {
    if (layer == utils::MapLayer::kBusLines) {
      proto_settings.add_layers(::TransportCatalog::MapLayer::kBusLines);
    } else if (layer == utils::MapLayer::kBusLabels) {
      proto_settings.add_layers(::TransportCatalog::MapLayer::kBusLabels);
    } else if (layer == utils::MapLayer::kStopPoints) {
      proto_settings.add_layers(::TransportCatalog::MapLayer::kStopPoints);
    } else {
      proto_settings.add_layers(::TransportCatalog::MapLayer::kStopLabels);
    }
  }
  proto_settings.set_outer_margin(outer_margin);
  return proto_settings;
}
}
