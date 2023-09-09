#pragma once

#include <atomic>

namespace harmony::threads {

class SpinLock {
 public:
  bool TryLock();
  void Lock();

  void Unlock();

  // BasicLockable
  bool try_lock();
  void lock();

  void unlock();

 private:
  std::atomic<bool> locked_{false};
};

}  // namespace harmony::threads
