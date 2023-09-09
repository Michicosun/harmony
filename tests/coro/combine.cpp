#include <gtest/gtest.h>

#include <harmony/fwd.hpp>

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

TEST(Coroutines, AllExceptionImmediately) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<std::tuple<size_t, std::string>> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      throw std::runtime_error("error");  // thrown in starter thread
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

TEST(Coroutines, FirstSimple) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      co_await coro::Yield();  // change thread
      std::this_thread::sleep_for(500ms);
      co_return 1;
    };

    auto task2 = []() -> coro::Task<size_t> {
      co_await coro::Yield();  // change thread
      std::this_thread::sleep_for(200ms);
      co_return 2;
    };

    co_return co_await coro::First(task1(), task2());
  };

  auto result = coro::Run(amain());

  ASSERT_EQ(result, 2);

  scheduler.WaitIdle();
  scheduler.Stop();
}

TEST(Coroutines, FirstException) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  auto amain = [&]() -> coro::Task<size_t> {
    co_await coro::Schedule(scheduler);

    auto task1 = []() -> coro::Task<size_t> {
      co_await coro::Yield();
      throw std::runtime_error("error");
      co_return 1;
    };

    auto task2 = []() -> coro::Task<size_t> {
      throw std::runtime_error("error");  // thrown in starter thread
      co_return 2;
    };

    co_return co_await coro::First(task1(), task2());
  };

  ASSERT_THROW(coro::Run(amain()), std::runtime_error);

  scheduler.WaitIdle();
  scheduler.Stop();
}

TEST(Coroutines, TaskGroupSimple) {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.Start();

  struct Result {
    std::chrono::milliseconds ellapsed;
    size_t done;
  };

  auto amain = [&]() -> coro::Task<Result> {
    co_await coro::Schedule(scheduler);

    auto start_t = std::chrono::steady_clock::now();
    std::atomic<size_t> done;

    auto sleeper = [&]() -> coro::Task<> {
      co_await coro::Yield();
      std::this_thread::sleep_for(50ms);
      done.fetch_add(1);
      co_return {};
    };

    coro::TaskGroup tg;
    for (size_t i = 0; i < 10; ++i) {
      co_await tg.Start(sleeper());
    }

    co_await tg.Wait();

    auto d1 = std::chrono::steady_clock::now() - start_t;
    auto d2 = std::chrono::duration_cast<std::chrono::milliseconds>(d1);

    co_return Result{
        .ellapsed = d2,
        .done = done.load(),
    };
  };

  auto result = coro::Run(amain());
  ASSERT_LE(result.ellapsed, 60ms);
  ASSERT_EQ(result.done, 10);

  scheduler.WaitIdle();
  scheduler.Stop();
}
