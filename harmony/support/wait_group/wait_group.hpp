#pragma once

#include <atomic>

namespace harmony::support {

class WaitGroup {
 public:
  void Add(size_t count);
  void Done();

  void Wait();

 private:
  std::atomic<uint32_t> size_{0};
};

}  // namespace harmony::support
