#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#include <harmony/threads/futex/futex.hpp>

TEST(FutexTest, Simple) {
  std::atomic<uint32_t> futex_cell = 0;
  auto key = harmony::threads::futex::PrepareWake(futex_cell);

  bool done = false;

  auto waiter = std::thread([&]() {
    harmony::threads::futex::Wait(futex_cell, 0);
    done = true;

    futex_cell.store(2);
    harmony::threads::futex::WakeOne(key);
  });

  ASSERT_FALSE(done);

  futex_cell.store(1);
  harmony::threads::futex::WakeOne(key);
  harmony::threads::futex::Wait(futex_cell, 1);

  ASSERT_TRUE(done);

  waiter.join();
}
