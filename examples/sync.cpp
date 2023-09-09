#include <iostream>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&](size_t coro_count) -> coro::Task<int> {
    co_await coro::Schedule(scheduler);

    coro::TaskGroup tg;
    coro::Mutex mutex;
    size_t counter = 0;

    auto runner = [&]() -> coro::Task<> {
      co_await coro::Yield();

      for (size_t j = 0; j < 100000; ++j) {
        auto lock = co_await mutex.ScopedLock();
        counter += 1;
      }

      co_return {};
    };

    for (size_t i = 0; i < coro_count; ++i) {
      co_await tg.Start(runner());
    }

    co_await tg.Wait();
    co_return counter;
  };

  std::cout << "DONE: " << coro::Run(amain(100)) << std::endl;

  scheduler.WaitIdle();
  scheduler.Stop();
  return 0;
}
