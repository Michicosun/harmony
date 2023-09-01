#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace harmony::support {

class RefCounter {
  class RefHandler {
   public:
    explicit RefHandler(RefCounter* counter)
        : counter_{counter} {
      counter->IncRefCount(1);
    }

    ~RefHandler() {
      counter_->DecRefCount();
    }

   private:
    RefCounter* counter_{nullptr};
  };

 public:
  void IncRefCount(size_t val) {
    ref_count_.fetch_add(val);
  }

  void DecRefCount() {
    if (ref_count_.fetch_sub(1) == 1) {
      delete this;
    }
  }

  RefHandler ScopedRef() {
    return RefHandler{this};
  }

  virtual ~RefCounter() = default;

 private:
  std::atomic<uint64_t> ref_count_{0};
};

}  // namespace harmony::support
