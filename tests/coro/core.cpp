#include <gtest/gtest.h>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>
#include <harmony/coro/run/run.hpp>
#include <harmony/coro/run/schedule.hpp>
#include <harmony/coro/run/yield.hpp>
#include <harmony/runtime/executors/compute/executor.hpp>
#include <harmony/runtime/executors/manual/executor.hpp>
#include <harmony/runtime/scheduler.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

TEST(Coroutines, Simple) {
  auto done = false;

  auto coroutine = [&]() -> coro::Task<std::monostate> {
    done = true;
    co_return std::monostate{};
  };

  auto task = coroutine();

  ASSERT_FALSE(done);

  coro::Detach(std::move(task));

  ASSERT_TRUE(done);
}

TEST(Coroutines, RunSimple) {
  auto coroutine = [&]() -> coro::Task<size_t> {
    co_return 42;
  };

  size_t x = coro::Run(coroutine());
  ASSERT_EQ(x, 42);
}

TEST(Coroutines, RunInScheduler) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(4);
  scheduler.Start();

  auto coroutine = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    std::this_thread::sleep_for(200ms);

    co_return 42;
  };

  size_t x = coro::Run(coroutine());
  ASSERT_EQ(x, 42);

  scheduler.WaitIdle();
  scheduler.Stop();
}

TEST(Coroutines, Schedule) {
  runtime::Scheduler<executors::ManualExecutor> scheduler;

  bool done = false;

  auto coroutine = [&]() -> coro::Task<std::monostate> {
    co_await coro::Schedule(scheduler);
    done = true;
    co_return std::monostate{};
  };

  coro::Detach(coroutine());

  ASSERT_FALSE(done);

  ASSERT_TRUE(scheduler.GetExecutor().RunNext());

  ASSERT_TRUE(done);
}

TEST(Coroutines, Yield) {
  runtime::Scheduler<executors::ManualExecutor> scheduler;

  bool done = false;

  auto coroutine = [&]() -> coro::Task<std::monostate> {
    co_await coro::Schedule(scheduler);

    for (size_t i = 0; i < 10; ++i) {
      co_await coro::Yield();
    }

    done = true;

    co_return std::monostate{};
  };

  coro::Detach(coroutine());

  size_t steps = scheduler.GetExecutor().Drain();
  ASSERT_EQ(steps, 1 + 10);

  ASSERT_TRUE(done);
}
