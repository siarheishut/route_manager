#include "request_processor.h"
#include "request_reader.h"

#include <iostream>
#include <utility>

int main() {
  using namespace rm;

  auto input = ReadInputRequests(std::cin);
  auto output = ReadOutputRequests(std::cin);
  if (!input || !output) return -1;

  BusManager bm(std::move(*input));

  for (auto &req : *output)
    ProcessRequest(bm, std::move(req), std::cout);
  return 0;
}
