#include "request_processor.h"
#include "request_reader.h"

#include <iostream>

int main() {
  using namespace rm;

  BusManager bm(ReadInputRequests(std::cin));

  for (auto &request : ReadOutputRequests(std::cin)) {
    ProcessRequest(bm, request, std::cout);
  }

  return 0;
}
