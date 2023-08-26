#include <atomic>
#include <cstddef>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <harmony/support/event/event.hpp>
#include <harmony/support/queues/blocking_queue.hpp>

class TestNode : public harmony::support::ForwardListNode<TestNode> {
 public:
  explicit TestNode(size_t x)
      : x_{x} {
  }

  size_t Destroy() const noexcept {
    size_t x = x_;
    delete this;
    return x;
  }

 private:
  size_t x_;
};

TEST(BlockingQueueTest, Simple) {
  harmony::support::UnboundedBlockingQueue<TestNode> queue;

  ASSERT_TRUE(queue.Push(new TestNode(7)));

  auto value = queue.TakeOne();
  ASSERT_TRUE(value);
  ASSERT_EQ(value.value()->Destroy(), 7);

  queue.Close();
  ASSERT_FALSE(queue.TakeOne());
}

TEST(BlockingQueueTest, Race) {
  harmony::support::UnboundedBlockingQueue<TestNode> queue;
  harmony::support::MPSCEvent event;

  const size_t max_concurrent_threads = std::thread::hardware_concurrency();
  const size_t produce_count = 10'000;
  const size_t group = std::max(1ul, max_concurrent_threads / 2);

  std::atomic<size_t> sum = 0;
  std::atomic<size_t> producers_running = group;
  std::vector<std::thread> consumers;
  std::vector<std::thread> producers;

  for (size_t i = 0; i < group; ++i) {
    producers.push_back(std::thread([&]() {
      for (size_t j = 1; j <= produce_count; ++j) {
        queue.Push(new TestNode(j));
      }

      if (producers_running.fetch_sub(1) == 1) {
        event.Complete();
      }
    }));
  }

  for (size_t i = 0; i < group; ++i) {
    consumers.push_back(std::thread([&]() {
      while (auto task = queue.TakeOne()) {
        sum.fetch_add(task.value()->Destroy());
      }
    }));
  }

  event.Wait();

  for (auto&& producer : producers) {
    producer.join();
  }

  queue.Close();

  for (auto&& consumer : consumers) {
    consumer.join();
  }

  ASSERT_EQ(sum.load(), (1 + produce_count) * produce_count / 2 * group);
}
