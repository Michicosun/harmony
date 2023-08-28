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

int64_t counter = 0;

coro::Task<std::monostate> contender(auto& scheduler, auto& mutex) {
  co_await coro::Schedule(scheduler);

  for (size_t i = 0; i < 100000; ++i) {
    auto lock = co_await mutex.ScopedLock();
    counter += 1;
  }

  co_return std::monostate{};
}

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto main = [&](size_t workers_count) -> coro::Task<int> {
    co_await coro::Schedule(scheduler);

    coro::Mutex mutex;

    std::cout << &counter << std::endl;

    for (size_t i = 0; i < workers_count; ++i) {
      coro::Detach(contender(scheduler, mutex));
    }

    std::this_thread::sleep_for(5s);

    co_return counter;
  };

  std::cout << coro::Run(main(10)) << std::endl;

  scheduler.Stop();
  return 0;
}
