#include <iostream>

#include "json.h"

#include "request_parser.h"
#include "request_processor.h"

int main() {
  json::Node root;
  std::cin >> root;
  json::Dict input_map;
  if (!root.IsMap()) return 1;
  input_map = root.ReleaseMap();

  auto base_requests = input_map.find("base_requests");
  auto stat_requests = input_map.find("stat_requests");
  auto routing_settings_it = input_map.find("routing_settings");

  if (base_requests == input_map.end() ||
      stat_requests == input_map.end() ||
      routing_settings_it == input_map.end())
    return -1;
  if (!base_requests->second.IsArray() ||
      !stat_requests->second.IsArray() ||
      !routing_settings_it->second.IsMap())
    return -1;

  auto input = rm::ParseInput(base_requests->second.ReleaseArray());
  auto
      output = rm::ParseOutput(stat_requests->second.ReleaseArray());
  auto routing_settings =
      rm::ParseRoutingSettings(routing_settings_it->second.ReleaseMap());
  if (!input || !output || !routing_settings) return 1;

  auto bm = rm::BusManager::Create(std::move(*input), *routing_settings);
  if (!bm) return -1;
  std::cout << rm::ProcessRequests(*bm, std::move(*output));

  return 0;
}
