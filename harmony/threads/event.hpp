#pragma once

#include <atomic>

#include <harmony/threads/futex.hpp>

namespace harmony::threads::sync {

class SPSCEvent {
  enum States : uint64_t {
    Empty = 0,
    Completed = 1,
  };

 public:
  void Wait();
  void Complete();

  // resets to empty
  // not thread_safe
  void Reset();

 private:
  std::atomic<uint32_t> completed_{States::Empty};
};

}  // namespace harmony::threads::sync
