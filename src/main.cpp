#include "request_processor.h"
#include "request_reader.h"

#include <iostream>
#include <utility>

int main() {
  using namespace rm;

  auto input = ReadInputRequests(std::cin);
  if (!input) return 0;

  BusManager bm(std::move(*input));

  auto output = ReadOutputRequests(std::cin);
  if (!output) return 0;
  for (auto &req : std::move(*output))
    ProcessRequest(bm, req, std::cout);
  return 0;
}
