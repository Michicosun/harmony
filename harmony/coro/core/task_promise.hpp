#pragma once

#include <harmony/coro/core/base_promise.hpp>
#include <harmony/result/result.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro {

template <class T>
class Task;

template <class T>
class TaskPromise : public BasePromise {
  using handle = std::coroutine_handle<TaskPromise>;

  struct FinalAwaiter {
    bool await_ready() noexcept {
      return false;
    }

    auto await_suspend(handle coroutine) noexcept {
      return coroutine.promise().ReleaseContinuation();
    }

    void await_resume() noexcept {
    }
  };

 public:
  Task<T> get_return_object() noexcept {
    return Task<T>(handle::from_promise(*this));
  }

  std::suspend_always initial_suspend() noexcept {
    return {};
  }

  void return_value(T arg) {
    result_.SetValue(std::move(arg));
  }

  void return_value(auto arg) {
    result_.SetValue(std::move(arg));
  }

  void unhandled_exception() noexcept {
    result_.SetException(std::current_exception());
  }

  FinalAwaiter final_suspend() noexcept {
    return {};
  }

 private:
  void SetContinuation(std::coroutine_handle<> continuation) {
    continuation_ = std::move(continuation);
  }

  std::coroutine_handle<> ReleaseContinuation() {
    return std::exchange(continuation_, {});
  }

  bool HasResult() const {
    return result_.has_value();
  }

  T UnwrapResult() {
    return result_.Unwrap();
  }

 private:
  friend class Task<T>;

  result::Result<T> result_;
  std::coroutine_handle<> continuation_ = std::noop_coroutine();
};

}  // namespace harmony::coro
