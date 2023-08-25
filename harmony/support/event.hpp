#pragma once

#include <atomic>

namespace harmony::support {

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

}  // namespace harmony::support
