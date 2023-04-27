#include "request_processor.h"
#include "request_reader.h"

#include <iostream>

int main() {
  using namespace rm;

  auto input = ReadInputRequests(std::cin);
  auto output = ReadOutputRequests(std::cin);
  if (!input || !output) return -1;

  auto bm = BusManager::Create(*input);
  if (!bm) return -1;

  for (auto &req : *output)
    ProcessRequest(*bm, req, std::cout);
  return 0;
}
