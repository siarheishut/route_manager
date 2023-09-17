#ifndef ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
#define ROOT_MANAGER_SRC_COORDS_CONVERTER_H_

#include <utility>

#include "svg/common.h"

#include "sphere.h"

namespace rm {
class CoordsConverter {
 public:
  struct Config {
    double min_lon, max_lon;
    double min_lat, max_lat;
    double width, height;
    double padding;
  };

  explicit CoordsConverter(const Config &config);

  svg::Point Convert(sphere::Coords coords) const;

 private:
  const double zoom_;
  const svg::Point center_;
};
}

#endif //ROOT_MANAGER_SRC_COORDS_CONVERTER_H_
