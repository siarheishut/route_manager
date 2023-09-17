#include "coords_converter.h"

#include <algorithm>

#include "svg/common.h"

#include "sphere.h"

namespace {
double Zoom(const rm::CoordsConverter::Config &c) {
  double width = c.width - 2 * c.padding;
  double height = c.height - 2 * c.padding;

  double dlon = c.max_lon - c.min_lon;
  double dlat = c.max_lat - c.min_lat;

  if (dlon == 0.0 && dlat == 0.0) return 0;
  if (dlon == 0.0) return height / dlat;
  if (dlat == 0.0) return width / dlon;
  return std::min(width / dlon, height / dlat);
}

svg::Point Center(const rm::CoordsConverter::Config &c, double zoom) {
  return {.x = c.padding - c.min_lon * zoom, .y = c.padding + c.max_lat * zoom};
}
}

namespace rm {
CoordsConverter::CoordsConverter(const CoordsConverter::Config &config)
    : zoom_(Zoom(config)), center_(Center(config, zoom_)) {
}

svg::Point CoordsConverter::Convert(sphere::Coords coords) const {
  return svg::Point{
      .x = center_.x + coords.longitude * zoom_,
      .y = center_.y - coords.latitude * zoom_};
}
}
