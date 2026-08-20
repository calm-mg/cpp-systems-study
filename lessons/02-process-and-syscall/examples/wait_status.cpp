#include "wait_status.hpp"

#include <sys/wait.h>

ChildStatus decode_wait_status(int status) noexcept {
  if (WIFEXITED(status)) {
    return ChildStatus{ChildState::exited, WEXITSTATUS(status)};
  }
  if (WIFSIGNALED(status)) {
    return ChildStatus{ChildState::signaled, WTERMSIG(status)};
  }
  return ChildStatus{ChildState::other, status};
}
