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

  auto base_requests_it = input_map.find("base_requests");
  auto stat_requests_it = input_map.find("stat_requests");
  auto routing_settings_it = input_map.find("routing_settings");

  if (base_requests_it == input_map.end() ||
      stat_requests_it == input_map.end() ||
      routing_settings_it == input_map.end())
    return -1;
  if (!base_requests_it->second.IsArray() ||
      !stat_requests_it->second.IsArray() ||
      !routing_settings_it->second.IsMap())
    return -1;

  auto base_requests = rm::ParseInput(base_requests_it->second.ReleaseArray());
  auto
      stat_requests = rm::ParseOutput(stat_requests_it->second.ReleaseArray());
  auto routing_settings =
      rm::ParseRoutingSettings(routing_settings_it->second.ReleaseMap());
  if (!base_requests || !stat_requests || !routing_settings) return 1;

  auto processor = rm::Processor::Create(
      std::move(*base_requests), *routing_settings);
  if (!processor) return -1;
  std::cout << processor->Process(*stat_requests);

  return 0;
}
