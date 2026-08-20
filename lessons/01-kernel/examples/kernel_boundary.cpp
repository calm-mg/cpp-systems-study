#include "process_info.hpp"

#include <exception>
#include <iostream>

int main() {
  try {
    const auto info = systems_study::read_process_info();

    std::cout
        << "User-space C++ requested kernel-managed process information.\n"
        << "process_id=" << info.process_id << '\n'
        << "parent_process_id=" << info.parent_process_id << '\n'
        << "page_size=" << info.page_size << '\n';
  } catch (const std::exception &error) {
    std::cerr << "Failed to read process information: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
