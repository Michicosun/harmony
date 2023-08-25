#include <cstddef>
#include <variant>

#include <fmt/core.h>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>
#include "harmony/coro/run/run.hpp"

harmony::coro::Task<std::monostate> boo(size_t i) {
  fmt::println("run: #{}", i);
  co_return std::monostate{};
}

harmony::coro::Task<size_t> foo() {
  for (size_t i = 0; i < 10; ++i) {
    co_await boo(i);
  }

  co_return 42;
}

void RunSync(auto task) {
  size_t x = harmony::coro::Run(task);
  fmt::println("returned: {}", x);
}

void RunDetached(auto task) {
  harmony::coro::Detach(std::move(task));
}

int main() {
  auto task = foo();
  RunSync(std::move(task));
  return 0;
}
