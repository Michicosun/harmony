#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>

#include <harmony/fwd.hpp>
#include <harmony/net/acceptor.hpp>
#include <ostream>
#include <thread>
#include <variant>
#include "harmony/coro/combine/first.hpp"
#include "harmony/coro/combine/task_group.hpp"
#include "harmony/coro/core/task.hpp"
#include "harmony/coro/events/sleep_for.hpp"
#include "harmony/coro/run/yield.hpp"
#include "harmony/runtime/io/core/fd.hpp"
#include "harmony/runtime/timers/core/deadline.hpp"

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithTimer();
  // scheduler.WithIO();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      auto sleeper = [](timers::Duration dur) -> coro::Task<std::monostate> {
        co_await coro::SleepFor(dur);
        std::cout << "done" << std::endl;
        co_return std::monostate{};
      };

      coro::TaskGroup tg =
          co_await coro::WithTaskGroup(sleeper(500ms), sleeper(1000ms));
      co_await tg.Wait();

      co_return 10;
    };

    auto task2 = []() -> coro::Task<size_t> {
      co_await coro::SleepFor(5000ms);
      co_return 10;
    };

    co_return co_await coro::First(task1(), task2());
  };

  std::cout << coro::Run(amain()) << std::endl;

  scheduler.WaitIdle();
  scheduler.Stop();

  return 0;
}
