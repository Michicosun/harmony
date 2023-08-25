#include <cstddef>
#include <variant>

#include <fmt/core.h>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>
#include <harmony/coro/run/run.hpp>

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

size_t SyncLoop() {
  size_t x = 0;

  for (size_t i = 0; i < 10; ++i) {
    x += harmony::coro::Run(foo());
  }

  return x;
}

void RunSync(auto task) {
  size_t x = harmony::coro::Run(task);
  fmt::println("returned: {}", x);
}

void RunDetached(auto task) {
  harmony::coro::Detach(std::move(task));
}

int main() {
  fmt::println("{}", SyncLoop());
  return 0;
}
