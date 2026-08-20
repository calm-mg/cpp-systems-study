#include "process_info.hpp"

#include <cerrno>
#include <system_error>

#include <unistd.h>

namespace systems_study {

ProcessInfo read_process_info() {
  errno = 0;
  const long page_size = ::sysconf(_SC_PAGESIZE);

  if (page_size <= 0) {
    const int error_number = errno == 0 ? EINVAL : errno;
    throw std::system_error(error_number, std::generic_category(),
                            "sysconf(_SC_PAGESIZE)");
  }

  return ProcessInfo{
      .process_id = static_cast<std::int64_t>(::getpid()),
      .parent_process_id = static_cast<std::int64_t>(::getppid()),
      .page_size = static_cast<std::int64_t>(page_size),
  };
}

} // namespace systems_study
