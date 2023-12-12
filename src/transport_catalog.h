#ifndef ROOT_MANAGER_SRC_TRANSPORT_CATALOG_H_
#define ROOT_MANAGER_SRC_TRANSPORT_CATALOG_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "common.h"
#include "request_types.h"
#include "sphere.h"

namespace rm {
class TransportCatalog {
 public:
  static std::unique_ptr<TransportCatalog> Create(
      std::vector<utils::PostRequest> requests);

  const utils::StopDict &Stops() const;

  const utils::BusDict &Buses() const;

 private:
  explicit TransportCatalog(std::vector<utils::PostRequest> request);

  void AddStop(std::string stop, sphere::Coords coords,
               const std::map<std::string, int> &stops);

  void AddBus(std::string bus, std::vector<std::string> stops,
              std::unordered_set<std::string> endpoints);

  utils::StopDict stops_;
  utils::BusDict buses_;
};
}

#endif // ROOT_MANAGER_SRC_TRANSPORT_CATALOG_H_
