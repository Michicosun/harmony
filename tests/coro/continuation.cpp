#include <gtest/gtest.h>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT

coro::Task<> boo() {
  co_await coro::Yield();
  co_return {};
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
