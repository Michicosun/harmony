#include <gtest/gtest.h>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>
#include <harmony/coro/run/run.hpp>
#include <harmony/coro/run/schedule.hpp>
#include <harmony/coro/run/yield.hpp>
#include <harmony/coro/sync/mutex.hpp>
#include <harmony/coro/sync/one_shot_event.hpp>
#include <harmony/coro/sync/wait_group.hpp>
#include <harmony/runtime/executors/compute/executor.hpp>
#include <harmony/runtime/scheduler.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

TEST(Coroutines, Event) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(4);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<bool> {
    co_await coro::Schedule(scheduler);

    bool done = false;
    coro::OneShotEvent event;

    auto coroutine = [&]() -> coro::Task<std::monostate> {
      co_await coro::Schedule(scheduler);

      std::this_thread::sleep_for(500ms);
      done = true;
      event.Complete();

      co_return std::monostate{};
    };

    coro::Detach(coroutine());
    co_await event.Wait();

    co_return done;
  };

  bool done = coro::Run(amain());
  ASSERT_TRUE(done);

  scheduler.WaitIdle();
  scheduler.Stop();
}

TEST(Coroutines, Mutex) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&](size_t coro_count) -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    coro::Mutex mutex;
    size_t counter = 0;

    coro::WaitGroup wg;
    wg.Add(coro_count);

    auto contender = [&]() -> coro::Task<std::monostate> {
      co_await coro::Schedule(scheduler);

      for (size_t j = 0; j < 100'000; ++j) {
        auto lock = co_await mutex.ScopedLock();
        counter += 1;
      }

      wg.Done();

      co_return std::monostate{};
    };

    for (size_t i = 0; i < coro_count; ++i) {
      coro::Detach(contender());
    }

    co_await wg.Wait();
    co_return counter;
  };

  size_t counter = coro::Run(amain(50));
  ASSERT_EQ(counter, 100'000 * 50);

  scheduler.WaitIdle();
  scheduler.Stop();
}
