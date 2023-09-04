#include <iostream>
#include <variant>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto main = [&](size_t coro_count) -> coro::Task<int> {
    co_await coro::Schedule(scheduler);

    coro::Mutex mutex;
    size_t counter = 0;

    coro::WaitGroup wg;
    wg.Add(coro_count);

    auto contender = [&]() -> coro::Task<std::monostate> {
      co_await coro::Schedule(scheduler);

      for (size_t j = 0; j < 100000; ++j) {
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

    std::cout << "DONE: " << counter << std::endl;

    co_return counter;
  };

  coro::Run(main(1000));

  scheduler.WaitIdle();
  scheduler.Stop();

  return 0;
}
