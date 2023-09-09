#pragma once

#include <atomic>
#include <cstdint>

namespace harmony::threads::futex {

struct WakeKey {
  uint32_t* addr;
};

// Wait
void Wait(std::atomic<uint32_t>& atomic, uint32_t old,
          std::memory_order mo = std::memory_order::seq_cst);

// Wake
WakeKey PrepareWake(std::atomic<uint32_t>& atomic);

void WakeOne(WakeKey key);
void WakeAll(WakeKey key);

}  // namespace harmony::threads::futex
