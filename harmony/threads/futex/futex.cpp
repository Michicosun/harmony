#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <harmony/threads/futex/futex.hpp>

namespace harmony::threads::futex {

namespace {

int futex(unsigned int* uaddr, int op, int val, const struct timespec* timeout,
          int* uaddr2, int val3) {
  return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}

uint32_t* ExtractPointer(std::atomic<uint32_t>& atomic) {
  return reinterpret_cast<uint32_t*>(&atomic);
}

}  // namespace

void Wait(std::atomic<uint32_t>& atomic, uint32_t old, std::memory_order mo) {
  while (atomic.load(mo) == old) {
    futex(ExtractPointer(atomic), FUTEX_WAIT_PRIVATE, old, nullptr, nullptr, 0);
  }
}

WakeKey PrepareWake(std::atomic<uint32_t>& atomic) {
  return {ExtractPointer(atomic)};
}

void WakeOne(WakeKey key) {
  futex(key.addr, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

void WakeAll(WakeKey key) {
  futex(key.addr, FUTEX_WAKE_PRIVATE, INT32_MAX, nullptr, nullptr, 0);
}

}  // namespace harmony::threads::futex
