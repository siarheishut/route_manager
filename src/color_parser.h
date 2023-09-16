#ifndef ROOT_MANAGER_SRC_COLOR_PARSER_H_
#define ROOT_MANAGER_SRC_COLOR_PARSER_H_

#include "json.h"
#include "svg/common.h"

namespace rm {
bool IsColor(const json::Node &node);
svg::Color AsColor(json::Node node);
}

#endif // ROOT_MANAGER_SRC_COLOR_PARSER_H_
