#include <gtest/gtest.h>

#include <harmony/coro/combine/all.hpp>
#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/run.hpp>
#include <harmony/coro/run/schedule.hpp>
#include <harmony/coro/run/yield.hpp>
#include <harmony/runtime/executors/compute/executor.hpp>
#include <harmony/runtime/scheduler.hpp>

using namespace harmony;  // NOLINT
using namespace std::chrono_literals;

TEST(Coroutines, AllSimple) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<std::tuple<size_t, std::string>> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      co_await coro::Yield();  // change thread
      std::this_thread::sleep_for(200ms);
      co_return 10;
    };

    auto task2 = []() -> coro::Task<std::string> {
      co_await coro::Yield();  // change thread
      std::this_thread::sleep_for(500ms);
      co_return "hello";
    };

    co_return co_await coro::All(task1(), task2());
  };

  auto result = coro::Run(amain());

  ASSERT_EQ(std::get<0>(result), 10);
  ASSERT_EQ(std::get<1>(result), "hello");

  scheduler.WaitIdle();
  scheduler.Stop();
}

TEST(Coroutines, AllException) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<std::tuple<size_t, std::string>> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      co_await coro::Yield();  // change thread
      throw std::runtime_error("error");
      co_return 10;
    };

    auto task2 = []() -> coro::Task<std::string> {
      co_await coro::Yield();  // change thread
      std::this_thread::sleep_for(500ms);
      co_return "hello";
    };

    co_return co_await coro::All(task1(), task2());
  };

  ASSERT_THROW(coro::Run(amain()), std::runtime_error);

  scheduler.WaitIdle();
  scheduler.Stop();
}
