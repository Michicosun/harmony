#pragma once

#include <coroutine>

namespace harmony::coro::concepts {

template <class T>
concept Awaiter = requires(T type, std::coroutine_handle<> c) {
  { type.await_ready() } -> std::same_as<bool>;
  // TODO: add await_suspend check
  { type.await_resume() };
};

}  // namespace harmony::coro::concepts
