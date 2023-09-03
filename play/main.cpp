#include <chrono>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>

#include <harmony/coro/combine/all.hpp>
#include <harmony/coro/combine/first.hpp>
#include <harmony/coro/core/task.hpp>
#include <harmony/coro/core/task_parameters.hpp>
#include <harmony/coro/core/task_promise.hpp>
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

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      throw std::runtime_error("error");  // thrown in starter thread
      co_return 1;
    };

    auto task2 = []() -> coro::Task<size_t> {
      throw std::runtime_error("error");  // thrown in starter thread
      co_return 2;
    };

    co_return co_await coro::First(task1(), task2());
  };

  coro::Run(amain());

  scheduler.WaitIdle();
  scheduler.Stop();

  return 0;
}
