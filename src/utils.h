#ifndef ROOT_MANAGER_SRC_UTILS_H_
#define ROOT_MANAGER_SRC_UTILS_H_

#include <string_view>

namespace rm {
std::string_view TrimStart(std::string_view sv);

std::string_view TrimEnd(std::string_view sv);

std::string_view ReadNextToken(std::string_view &sv,
                               std::string_view delim = " ");
}

#endif // ROOT_MANAGER_SRC_UTILS_H_
