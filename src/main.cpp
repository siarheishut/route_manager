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

  auto base_requests = input_map.find("base_requests");
  auto stat_requests = input_map.find("stat_requests");
  auto routing_settings = input_map.find("routing_settings");

  if (base_requests == input_map.end()
      || stat_requests == input_map.end()
      || routing_settings == input_map.end())
    return -1;
  if (!base_requests->second.IsArray() ||
      !stat_requests->second.IsArray() ||
      !routing_settings->second.IsMap())
    return -1;

  auto input = rm::ParseInput(base_requests->second.ReleaseArray());
  auto
      output = rm::ParseOutput(stat_requests->second.ReleaseArray());
  auto settings = rm::ParseSettings(routing_settings->second.ReleaseMap());
  if (!input || !output || !settings) return 1;

  auto bm = rm::BusManager::Create(std::move(*input), *settings);
  if (!bm) return -1;
  std::cout << rm::ProcessRequests(*bm, std::move(*output));

  return 0;
}
