#include <exception>
#include <iostream>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/run/run.hpp>
#include <stdexcept>
#include <variant>

harmony::coro::Task<std::string> boo(int x) {
  if (x % 3 == 0) {
    throw std::runtime_error("hello");
  }

  co_return "bye";
}

harmony::coro::Task<int> foo(int x) {
  std::cout << "running boo..." << std::endl;

  std::string s = co_await boo(x);
  std::cout << "boo return: " << s << std::endl;

  co_return 10;
}

harmony::coro::Task<std::monostate> amain() {
  try {
    int x;
    std::cin >> x;

    co_await foo(x);
  } catch (const std::exception& e) {
    std::cout << "caught: " << e.what() << std::endl;
  }

  co_return std::monostate{};
}

int main() {
  harmony::coro::Run(amain());
  return 0;
}
