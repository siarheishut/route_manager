#include "utils.h"

#include <string_view>

namespace rm {
std::string_view TrimStart(std::string_view sv) {
  auto pos = sv.find_first_not_of(' ');
  if (pos == sv.npos) pos = sv.size();
  sv.remove_prefix(pos);
  return sv;
}

std::string_view TrimEnd(std::string_view sv) {
  auto pos = sv.find_last_not_of(' ');
  if (pos == sv.npos) return "";
  sv.remove_suffix(sv.length() - pos - 1);
  return sv;
}

// If empty "" delim is provided, just trims the string.
std::string_view ReadNextToken(std::string_view &sv,
                               std::string_view delim) {
  sv = TrimStart(sv);
  auto pos = sv.find_first_of(delim);
  auto token = TrimEnd(sv.substr(0, pos));
  if (pos != sv.npos) ++pos;
  else pos = sv.size();
  sv.remove_prefix(pos);
  sv = TrimStart(sv);
  return token;
}
}
