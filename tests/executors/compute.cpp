#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <harmony/executors/compute/executor.hpp>
#include <harmony/executors/task.hpp>

using namespace std::chrono_literals;

struct WaitTask : public harmony::executors::TaskBase {
  bool& flag;
  std::chrono::milliseconds timeout;

  WaitTask(bool& flag, std::chrono::milliseconds timeout)
      : flag{flag},
        timeout{std::move(timeout)} {
  }

  void Run() noexcept override {
    std::this_thread::sleep_for(std::move(timeout));
    flag = true;
    delete this;
  }
};

struct IncTask : public harmony::executors::TaskBase {
  std::atomic<uint32_t>& cell;

  explicit IncTask(std::atomic<uint32_t>& cell)
      : cell{cell} {
  }

  void Run() noexcept override {
    cell.fetch_add(1);
    delete this;
  }
};

TEST(ComputeExecutorTest, Wait) {
  harmony::executors::ComputeExecutor pool{4};
  pool.Start();

  bool done;
  pool.Submit(new WaitTask(done, 200ms));

  pool.WaitIdle();
  pool.Stop();
}

TEST(ComputeExecutorTest, MultiWait) {
  harmony::executors::ComputeExecutor pool{1};
  pool.Start();

  for (size_t i = 0; i < 3; ++i) {
    bool done = false;

    pool.Submit(new WaitTask(done, 200ms));

    pool.WaitIdle();
    ASSERT_TRUE(done);
  }

  pool.Stop();
}

TEST(ComputeExecutorTest, ManyTasks) {
  harmony::executors::ComputeExecutor pool{4};
  pool.Start();

  static const size_t kTasks = 170;
  std::atomic<uint32_t> tasks{0};

  for (size_t i = 0; i < kTasks; ++i) {
    pool.Submit(new IncTask(tasks));
  }

  pool.WaitIdle();
  pool.Stop();

  ASSERT_EQ(tasks.load(), kTasks);
}
