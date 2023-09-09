#include <iostream>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithTimer();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    auto long_running_query = []() -> coro::Task<size_t> {
      co_await coro::SleepFor(10s);
      co_return 1;
    };

    auto timeout = []() -> coro::Task<size_t> {
      co_await coro::SleepFor(500ms);
      co_return 2;
    };

    co_return co_await coro::First(long_running_query(), timeout());
  };

  // will return 2, long running query cancelled
  std::cout << "RESULT: " << coro::Run(amain()) << std::endl;

  scheduler.Stop();
  return 0;
}
