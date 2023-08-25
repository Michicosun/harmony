#pragma once

#include <exception>
#include <coroutine>
#include <utility>

#include <harmony/coro/core/task_promise.hpp>

namespace harmony::coro {

template <class T>
class Task {
  struct TaskAwaiter {
    explicit TaskAwaiter(std::coroutine_handle<TaskPromise<T>> h) noexcept
        : coro_(h) {
    }

    bool await_ready() noexcept {
      return !coro_ || coro_.done();
    }

    auto await_suspend(std::coroutine_handle<> continuation) noexcept {
      coro_.promise().SetContinuation(std::move(continuation));
      return coro_;
    }

    T await_resume() noexcept {
      return coro_.promise().UnwrapResult();
    }

    std::coroutine_handle<TaskPromise<T>> coro_;
  };

 public:
  using promise_type = TaskPromise<T>;

  Task(Task&& t) noexcept
      : coro_(std::exchange(t.coro_, {})) {
  }

  ~Task() {
    if (coro_ && !coro_.done()) {
      std::terminate();
    }

    if (coro_ && coro_.done()) {
      coro_.destroy();
    }
  }

 public:
  auto operator co_await() const& noexcept {
    return TaskAwaiter{coro_};
  }

 private:
  friend class TaskPromise<T>;

  explicit Task(std::coroutine_handle<promise_type> h) noexcept
      : coro_(h) {
  }

 private:
  std::coroutine_handle<promise_type> coro_;
};

}  // namespace harmony::coro
