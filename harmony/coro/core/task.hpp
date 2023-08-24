#pragma once

#include <exception>
#include <coroutine>

#include <harmony/coro/core/task_promise.hpp>
#include <stdexcept>

namespace harmony::coro {

template <class T>
class Task {
 public:
  using promise_type = TaskPromise<T>;

  Task(Task&& t) noexcept
      : coro_(std::exchange(t.coro_, {})) {
  }

  ~Task() {
    if (coro_.done()) {
      coro_.destroy();
    } else {
      std::terminate();
    }
  }

  bool await_ready() noexcept {
    return coro_.promise().HasResult();
  }

  auto await_suspend(std::coroutine_handle<> continuation) noexcept {
    coro_.promise().SetContinuation(std::move(continuation));
    return coro_;
  }

  T await_resume() noexcept {
    return coro_.promise().UnwrapResult();
  }

 private:
  friend class TaskPromise<T>;

  explicit Task(std::coroutine_handle<promise_type> h) noexcept
      : coro_(h) {
  }

 private:
  std::coroutine_handle<TaskPromise<T>> coro_;
};

}  // namespace harmony::coro
