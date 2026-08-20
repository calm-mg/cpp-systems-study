#pragma once

enum class ChildState {
  exited,
  signaled,
  other,
};

struct ChildStatus {
  ChildState state;
  int detail;
};

ChildStatus decode_wait_status(int status) noexcept;
