#include "json.h"
#include "request_processor.h"
#include "request_parser.h"

#include <iostream>

int main() {
  json::Node root;
  std::cin >> root;
  json::Dict input_map;
  if (!root.IsMap()) return 1;
  input_map = root.ReleaseMap();

  if (input_map.count("base_requests") == 0 ||
      input_map.count("stat_requests") == 0) {
    return 1;
  }

  auto base_requests = input_map.find("base_requests");
  auto stat_requests = input_map.find("stat_requests");

  if (base_requests == input_map.end()
      || stat_requests == input_map.end())
    return -1;
  if (!base_requests->second.IsArray() ||
      !stat_requests->second.IsArray())
    return -1;

  auto input = rm::ParseInput(base_requests->second.ReleaseArray());
  auto
      output = rm::ParseOutput(stat_requests->second.ReleaseArray());
  if (!input || !output) return 1;

  auto bm = rm::BusManager::Create(std::move(*input));
  if (!bm) return -1;
  std::cout << rm::ProcessRequests(*bm, std::move(*output));

  return 0;
}
