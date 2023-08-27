#pragma once

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro::impl {

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

template <concepts::Awaitable Awaitable>
static DetachTask CreateDetachTask(Awaitable&& object) {
  auto saved_object = std::move(object);
  co_await saved_object;
  co_return;
}

}  // namespace harmony::coro::impl
