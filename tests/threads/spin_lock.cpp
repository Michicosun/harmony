#include <mutex>  // for lock_guard
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <harmony/threads/spin_lock.hpp>

#include <harmony/support/event.hpp>

TEST(SpinLockTest, Simple) {
  harmony::threads::SpinLock lock;
  harmony::support::SPSCEvent event;
  std::vector<std::thread> threads;

  const size_t spin_count = 10'000;
  const size_t threads_count = std::thread::hardware_concurrency();
  std::atomic<size_t> threads_running = threads_count;
  size_t sum = 0;

  for (size_t i = 0; i < threads_count; ++i) {
    threads.push_back(std::thread([&]() {
      for (size_t j = 0; j < spin_count; ++j) {
        std::lock_guard guard(lock);
        sum += 1;
      }

      if (threads_running.fetch_sub(1) == 1) {
        event.Complete();  // last worker
      }
    }));
  }

  event.Wait();

  ASSERT_EQ(sum, threads_count * spin_count);

  for (auto&& thread : threads) {
    thread.join();
  }
}
