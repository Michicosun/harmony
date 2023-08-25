#pragma once

#include <coroutine>

namespace harmony::coro::concepts {

template <class T>
concept AwaiterSuspend = requires(T type, std::coroutine_handle<> c) {
  std::same_as<decltype(type.await_suspend(c)), void> ||
      std::same_as<decltype(type.await_suspend(c)), bool> ||
      std::same_as<decltype(type.await_suspend(c)), std::coroutine_handle<>>;
};

template <class T>
concept Awaiter = requires(T type, std::coroutine_handle<> c) {
  { type.await_ready() } -> std::same_as<bool>;
  { type.await_resume() };
} && AwaiterSuspend<T>;

}  // namespace harmony::coro::concepts
