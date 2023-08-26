#include <exception>
#include <iostream>
#include <thread>
#include <variant>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>
#include <harmony/coro/run/run.hpp>
#include <harmony/coro/run/schedule.hpp>
#include <harmony/coro/run/yield.hpp>
#include <harmony/runtime/executors/manual/executor.hpp>

harmony::coro::Task<std::string> boo(
    int x, harmony::runtime::executors::ManualExecutor& executor) {
  std::cout << "running on thread: " << std::this_thread::get_id() << std::endl;

  if (x % 3 == 0) {
    co_await harmony::coro::Schedule(executor);
    std::cout << "scheduled inside" << std::endl;
    co_await harmony::coro::Yield();
  }

  std::cout << "running on thread: " << std::this_thread::get_id() << std::endl;

  co_return "bye";
}

harmony::coro::Task<int> foo(
    int x, harmony::runtime::executors::ManualExecutor& executor) {
  std::cout << "running boo..." << std::endl;

  std::string s = co_await boo(x, executor);
  std::cout << "boo return: " << s << std::endl;

  co_return 10;
}

harmony::coro::Task<std::monostate> amain(
    harmony::runtime::executors::ManualExecutor& executor) {
  try {
    int x;
    std::cin >> x;

    co_await foo(x, executor);
  } catch (const std::exception& e) {
    std::cout << "caught: " << e.what() << std::endl;
  }

  co_return std::monostate{};
}

harmony::coro::Task<int> sync() {
  co_return 10ll;
}

int main() {
  harmony::runtime::executors::ManualExecutor executor;

  std::cout << "sync: " << harmony::coro::Run(sync()) << std::endl;
  harmony::coro::Detach(amain(executor));

  std::cout << "before scheduling: " << executor.TasksInQueue() << std::endl;

  executor.RunNext();

  return 0;
}
