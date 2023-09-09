#pragma once

#include <coroutine>
#include <utility>

#include <harmony/coro/core/task_promise.hpp>
#include <harmony/result/unit.hpp>
#include <harmony/support/terminate/terminate.hpp>

namespace harmony::coro {

template <class T = result::Unit>
class Task {
  struct [[nodiscard]] TaskAwaiter {
    explicit TaskAwaiter(const Task<T>* task) noexcept
        : task_(task) {
    }

    bool await_ready() const noexcept {
      return !task_->coro_ || task_->coro_.done();
    }

    template <concepts::BasePromiseConvertible Promise>
    auto await_suspend(std::coroutine_handle<Promise> continuation) {
      BasePromise& promise = continuation.promise();
      parameters_ = &promise.GetParameters();

      if (parameters_->NeedCancel()) {
        task_->cancelled_ = true;
        ThrowCancel();
      }

      task_->coro_.promise().SetContinuation(std::move(continuation));
      return task_->coro_;
    }

    T await_resume() {
      CheckCancel(parameters_);
      return task_->coro_.promise().UnwrapResult();
    }

    const Task<T>* task_{nullptr};
    CoroParameters* parameters_{nullptr};
  };

 public:
  using promise_type = TaskPromise<T>;

  Task(Task&& t) noexcept
      : coro_(std::exchange(t.coro_, {})) {
  }

  ~Task() {
    if (coro_ && coro_.done()) {
      coro_.destroy();
    } else if (coro_ && cancelled_) {
      coro_.destroy();
    } else if (coro_ && !coro_.done()) {
      support::Terminate("task was destroyed before coroutine was completed");
    }
  }

 public:
  auto operator co_await() const& noexcept {
    return TaskAwaiter(this);
  }

  promise_type& GetPromise() {
    return coro_.promise();
  }

 private:
  friend class TaskPromise<T>;

  explicit Task(std::coroutine_handle<promise_type> h) noexcept
      : coro_(h) {
  }

 private:
  std::coroutine_handle<promise_type> coro_;
  mutable bool cancelled_{false};
};

}  // namespace harmony::coro
