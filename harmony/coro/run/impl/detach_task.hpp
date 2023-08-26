#pragma once

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro {

class DetachTask {
  class DetachPromise {
    using handle = std::coroutine_handle<DetachPromise>;

    struct FinalAwaiter {
      bool await_ready() noexcept;
      void await_suspend(handle coroutine) noexcept;
      void await_resume() noexcept;
    };

   public:
    DetachPromise() = default;

    DetachTask get_return_object() noexcept;
    std::suspend_always initial_suspend() noexcept;
    void return_void() noexcept;
    void unhandled_exception() noexcept;
    FinalAwaiter final_suspend() noexcept;
  };

 public:
  using promise_type = DetachPromise;

  void Start();

 private:
  explicit DetachTask(std::coroutine_handle<promise_type> h) noexcept;

 private:
  std::coroutine_handle<promise_type> coro_;
};

template <concepts::Awaitable awaitable>
static DetachTask CreateDetachTask(awaitable&& object) {
  co_await object;
  co_return;
}

}  // namespace harmony::coro
