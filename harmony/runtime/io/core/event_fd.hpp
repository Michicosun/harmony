#pragma once

#include <sys/eventfd.h>
#include <atomic>

#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::io {

class EventFd {
 public:
  EventFd();

  // sets efd = 1
  void Signal();

  // resets efd to 0
  // returns true if was signalled
  bool Reset();

  // returns underlying efd
  io::Fd NativeHandle() const;

 private:
  io::Fd efd_{0};
  std::atomic<bool> signalled_{false};
};

}  // namespace harmony::io
