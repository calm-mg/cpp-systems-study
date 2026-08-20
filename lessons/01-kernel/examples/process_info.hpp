#pragma once

#include <cstdint>

namespace systems_study {

struct ProcessInfo {
  std::int64_t process_id;
  std::int64_t parent_process_id;
  std::int64_t page_size;
};

[[nodiscard]] ProcessInfo read_process_info();

} // namespace systems_study
