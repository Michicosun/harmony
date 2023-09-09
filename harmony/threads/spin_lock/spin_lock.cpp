#include <thread>

#include <harmony/threads/spin_lock/spin_lock.hpp>

namespace harmony::threads {

void SpinLock::Lock() {
  while (locked_.exchange(true, std::memory_order::acquire)) {
    while (locked_.load(std::memory_order::relaxed)) {
      std::this_thread::yield();
    }
  }
}

bool SpinLock::TryLock() {
  return !locked_.exchange(true, std::memory_order::acquire);
}

void SpinLock::Unlock() {
  locked_.store(false, std::memory_order::release);
}

void SpinLock::lock() {
  Lock();
}

bool SpinLock::try_lock() {
  return TryLock();
}

void SpinLock::unlock() {
  Unlock();
}

}  // namespace harmony::threads
