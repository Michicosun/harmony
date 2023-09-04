#include <gtest/gtest.h>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

TEST(Coroutines, TimersSimple) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithTimer();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);
    co_await coro::SleepFor(100ms);
    co_return 0;
  };

  coro::Run(amain());

  scheduler.WaitIdle();
  scheduler.Stop();
}

TEST(Coroutines, TimersParallel) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithTimer();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    auto sleeper = [&](size_t i) -> coro::Task<size_t> {
      co_await coro::SleepFor(500ms);
      co_return i;
    };

    auto results = co_await coro::All(sleeper(1), sleeper(2), sleeper(3),
                                      sleeper(4), sleeper(5));

    co_return std::get<0>(results) + std::get<1>(results) +
        std::get<2>(results) + std::get<3>(results) + std::get<4>(results);
  };

  auto start_time = timers::Clock::now();

  size_t sum = coro::Run(amain());
  ASSERT_EQ(sum, 15);

  auto elapsed_time = timers::Clock::now() - start_time;
  ASSERT_LE(elapsed_time, 550ms);

  scheduler.WaitIdle();
  scheduler.Stop();
}
