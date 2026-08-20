#include "wait_status.hpp"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>

namespace {

int collect_child_status(void (*child_action)()) {
  const pid_t child_pid = ::fork();
  if (child_pid == -1) {
    std::perror("fork");
    std::exit(EXIT_FAILURE);
  }

  if (child_pid == 0) {
    child_action();
    ::_exit(EXIT_FAILURE);
  }

  int status = 0;
  pid_t result = -1;
  do {
    result = ::waitpid(child_pid, &status, 0);
  } while (result == -1 && errno == EINTR);

  if (result == -1) {
    std::perror("waitpid");
    std::exit(EXIT_FAILURE);
  }
  return status;
}

void exit_with_23() { ::_exit(23); }

void terminate_with_sigterm() {
  struct sigaction default_action{};
  default_action.sa_handler = SIG_DFL;
  if (sigemptyset(&default_action.sa_mask) == -1 ||
      ::sigaction(SIGTERM, &default_action, nullptr) == -1) {
    std::perror("reset SIGTERM");
    ::_exit(EXIT_FAILURE);
  }
  if (::raise(SIGTERM) != 0) {
    std::fputs("raise SIGTERM failed\n", stderr);
    ::_exit(EXIT_FAILURE);
  }
  ::_exit(EXIT_FAILURE);
}

bool expect_equal(int actual, int expected, std::string_view label) {
  if (actual == expected) {
    return true;
  }
  std::cerr << label << ": expected " << expected << ", got " << actual << '\n';
  return false;
}

} // namespace

int main() {
  bool passed = true;

  const ChildStatus exited =
      decode_wait_status(collect_child_status(exit_with_23));
  passed &= expect_equal(static_cast<int>(exited.state),
                         static_cast<int>(ChildState::exited), "exit state");
  passed &= expect_equal(exited.detail, 23, "exit code");

  struct sigaction ignored_action{};
  ignored_action.sa_handler = SIG_IGN;
  if (sigemptyset(&ignored_action.sa_mask) == -1) {
    std::perror("sigemptyset");
    return EXIT_FAILURE;
  }

  struct sigaction original_action{};
  if (::sigaction(SIGTERM, &ignored_action, &original_action) == -1) {
    std::perror("sigaction ignore SIGTERM");
    return EXIT_FAILURE;
  }

  const ChildStatus signaled =
      decode_wait_status(collect_child_status(terminate_with_sigterm));

  if (::sigaction(SIGTERM, &original_action, nullptr) == -1) {
    std::perror("sigaction restore SIGTERM");
    return EXIT_FAILURE;
  }
  passed &=
      expect_equal(static_cast<int>(signaled.state),
                   static_cast<int>(ChildState::signaled), "signal state");
  passed &= expect_equal(signaled.detail, SIGTERM, "signal number");

  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
