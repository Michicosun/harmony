#include <thread>

#include <gtest/gtest.h>

#include <harmony/support/event/event.hpp>

TEST(EventTest, Simple) {
  harmony::support::MPSCEvent event_1;
  harmony::support::MPSCEvent event_2;
  std::atomic<size_t> count = 0;

  auto thread_1 = std::thread([&]() {
    for (size_t i = 0; i < 10; ++i) {
      event_1.Wait();

      ASSERT_TRUE(count.load() == 2 * i + 1);
      count.fetch_add(1);

      event_1.Reset();
      event_2.Complete();
    }
  });

  auto thread_2 = std::thread([&]() {
    for (size_t i = 0; i < 10; ++i) {
      event_2.Reset();

      ASSERT_TRUE(count.load() == 2 * i);
      count.fetch_add(1);

      event_1.Complete();
      event_2.Wait();
    }
  });

  thread_1.join();
  thread_2.join();

  ASSERT_TRUE(count.load() == 20);
}
