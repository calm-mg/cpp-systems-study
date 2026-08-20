#include "wait_status.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>

namespace {

constexpr int kChildExitCode = 42;

std::string_view state_name(ChildState state) {
  switch (state) {
  case ChildState::exited:
    return "exited";
  case ChildState::signaled:
    return "signaled";
  case ChildState::other:
    return "other";
  }
  return "unknown";
}

int run_after_exec() {
  std::cout << "after_exec pid=" << ::getpid() << " program_image=replaced\n";
  return kChildExitCode;
}

int wait_for_child(pid_t child_pid) {
  int status = 0;
  pid_t result = -1;
  do {
    result = ::waitpid(child_pid, &status, 0);
  } while (result == -1 && errno == EINTR);

  if (result == -1) {
    std::perror("waitpid");
    return -1;
  }
  return status;
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc == 2 && std::string_view(argv[1]) == "--after-exec") {
    return run_after_exec();
  }
  if (argc != 1) {
    std::cerr << "usage: " << argv[0] << " [--after-exec]\n";
    return EXIT_FAILURE;
  }

  int value = 10;
  std::cout << "parent_before_fork pid=" << ::getpid() << " value=" << value
            << '\n'
            << std::flush;

  const pid_t child_pid = ::fork();
  if (child_pid == -1) {
    std::perror("fork");
    return EXIT_FAILURE;
  }

  if (child_pid == 0) {
    value = 20;
    std::cout << "child_before_exec pid=" << ::getpid()
              << " parent_pid=" << ::getppid() << " value=" << value << '\n'
              << std::flush;

    ::execlp(argv[0], argv[0], "--after-exec", static_cast<char *>(nullptr));
    std::perror("execlp");
    ::_exit(127);
  }

  const int raw_status = wait_for_child(child_pid);
  if (raw_status == -1) {
    return EXIT_FAILURE;
  }

  const ChildStatus child_status = decode_wait_status(raw_status);
  std::cout << "parent_after_wait pid=" << ::getpid()
            << " child_pid=" << child_pid << " value=" << value
            << " child_state=" << state_name(child_status.state)
            << " child_detail=" << child_status.detail << '\n';

  const bool lifecycle_is_expected = value == 10 &&
                                     child_status.state == ChildState::exited &&
                                     child_status.detail == kChildExitCode;
  return lifecycle_is_expected ? EXIT_SUCCESS : EXIT_FAILURE;
}
