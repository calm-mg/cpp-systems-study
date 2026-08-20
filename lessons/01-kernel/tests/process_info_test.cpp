#include "process_info.hpp"

#include <iostream>

int main() {
  const auto info = systems_study::read_process_info();

  if (info.process_id <= 0) {
    std::cerr << "process_id must be positive\n";
    return 1;
  }

  if (info.parent_process_id < 0) {
    std::cerr << "parent_process_id must not be negative\n";
    return 1;
  }

  if (info.page_size <= 0) {
    std::cerr << "page_size must be positive\n";
    return 1;
  }

  return 0;
}
