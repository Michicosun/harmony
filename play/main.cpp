#include <atomic>
#include <cassert>
#include <cstddef>
#include <thread>
#include <variant>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>
#include <harmony/coro/run/run.hpp>
#include <harmony/coro/run/schedule.hpp>
#include <harmony/coro/run/yield.hpp>
#include <harmony/coro/sync/mutex.hpp>
#include <harmony/coro/sync/one_shot_event.hpp>
#include <harmony/runtime/executors/compute/executor.hpp>
#include <harmony/runtime/scheduler.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto main = [&](size_t coro_count) -> coro::Task<int> {
    co_await coro::Schedule(scheduler);

    std::atomic<size_t> running = coro_count;
    coro::OneShotEvent event;
    coro::Mutex mutex;
    size_t counter = 0;

    auto contender = [&]() -> coro::Task<std::monostate> {
      co_await coro::Schedule(scheduler);

      for (size_t j = 0; j < 100000; ++j) {
        auto lock = co_await mutex.ScopedLock();
        counter += 1;
      }

      if (running.fetch_sub(1) == 1) {
        event.Complete();
      }

      co_return std::monostate{};
    };

    for (size_t i = 0; i < coro_count; ++i) {
      coro::Detach(contender());
    }

    co_await event.Wait();

    std::cout << "DONE: " << counter << std::endl;

    co_return counter;
  };

  coro::Run(main(100));

  scheduler.WaitIdle();
  scheduler.Stop();

  return 0;
}
