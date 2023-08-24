#include <cstddef>
#include <variant>

#include <fmt/core.h>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/detach.hpp>

harmony::coro::Task<std::monostate> boo(size_t i) {
  fmt::println("run: #{}", i);
  co_return std::monostate{};
}

harmony::coro::Task<std::monostate> foo() {
  for (size_t i = 0; i < 10'000'000; ++i) {
    co_await boo(i);
  }

  co_return std::monostate{};
}

int main() {
  harmony::coro::Detach(foo());
  return 0;
}
