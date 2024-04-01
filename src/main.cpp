#include <fstream>
#include <iostream>
#include <string_view>

#include "json.h"
#include "request_parser.h"
#include "request_processor.h"
#include "transport_catalog.pb.h"

using namespace std;

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: transport_catalog_part_o [make_base|process_requests]\n";
    return 5;
  }

  const string_view mode(argv[1]);

  if (mode == "make_base") {
    json::Node root;
    std::cin >> root;
    json::Dict input_map;
    if (!root.IsMap()) return 1;
    input_map = root.ReleaseMap();

    auto base_requests_it = input_map.find("base_requests");
    auto routing_settings_it = input_map.find("routing_settings");
    auto rendering_settings_it = input_map.find("render_settings");
    auto serialization_settings_it = input_map.find("serialization_settings");

    if (base_requests_it == input_map.end() ||
        routing_settings_it == input_map.end() ||
        rendering_settings_it == input_map.end() ||
        serialization_settings_it == input_map.end())
      return 2;

    if (!base_requests_it->second.IsArray() ||
        !routing_settings_it->second.IsMap() ||
        !rendering_settings_it->second.IsMap() ||
        !serialization_settings_it->second.IsMap())
      return 3;

    auto base_requests =
        rm::ParseInput(base_requests_it->second.ReleaseArray());
    auto routing_settings =
        rm::ParseRoutingSettings(routing_settings_it->second.AsMap());
    auto rendering_settings =
        rm::ParseRenderingSettings(rendering_settings_it->second.AsMap());
    auto serialization_settings = rm::ParseSerializationSettings(
        serialization_settings_it->second.ReleaseMap());
    if (!base_requests || !serialization_settings)
      return 4;

    auto processor = rm::Processor::Create(std::move(*base_requests),
                                           *routing_settings,
                                           *rendering_settings);
    if (!processor)
      return 5;

    auto proto_transport_catalog = processor->Serialize();
    *proto_transport_catalog.mutable_rendering_settings() =
        rendering_settings->Serialize();
    std::ofstream file(serialization_settings->file_name, std::ios::binary);
    if (!proto_transport_catalog.SerializeToOstream(&file))
      return 6;

  } else if (mode == "process_requests") {
    json::Node root;
    std::cin >> root;
    json::Dict input_map;
    if (!root.IsMap()) return 1;
    input_map = root.ReleaseMap();

    auto stat_requests_it = input_map.find("stat_requests");
    auto serialization_settings_it = input_map.find("serialization_settings");

    if (stat_requests_it == input_map.end() ||
        serialization_settings_it == input_map.end())
      return 1;

    if (!stat_requests_it->second.IsArray() ||
        !serialization_settings_it->second.IsMap())
      return 1;

    auto stat_requests =
        rm::ParseOutput(stat_requests_it->second.ReleaseArray());
    auto serialization_settings = rm::ParseSerializationSettings(
        serialization_settings_it->second.ReleaseMap());

    if (!stat_requests || !serialization_settings) return 1;

    std::ifstream file(serialization_settings->file_name, std::ios::binary);
    if (file.fail()) return 2;
    ::TransportCatalog::TransportCatalog proto_catalog;
    if (!proto_catalog.ParseFromIstream(&file))
      return 1;
    auto processor = rm::Processor::Deserialize(std::move(proto_catalog));
    if (!processor) {
      std::cerr << "Lol, minus processor 0-0";
      return -1;
    }
    std::cout << processor->Process(*stat_requests);
  }

  return 0;
}
