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

coro::Task<std::monostate> boo() {
  co_await coro::Yield();
  co_return std::monostate{};
}

coro::Task<size_t> foo(size_t x) {
  for (size_t i = 0; i < 100; ++i) {
    co_await boo();
  }

  size_t sqr = x * x;
  co_return sqr;
}

TEST(Coroutines, Continuation) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    size_t sum = 0;

    for (size_t i = 0; i < 100; ++i) {
      sum += co_await foo(i);
    }

    co_return sum;
  };

  size_t sum = coro::Run(amain());
  ASSERT_EQ(sum, 328350);

  scheduler.WaitIdle();
  scheduler.Stop();
}
