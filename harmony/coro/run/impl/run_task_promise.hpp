#pragma once

#include <coroutine>

#include <harmony/coro/core/base_promise.hpp>
#include <harmony/result/result.hpp>
#include <harmony/threads/event/event.hpp>

namespace harmony::coro::impl {

template <class T>
class RunTask;

template <class T>
class RunTaskPromise : public BasePromise {
  using handle = std::coroutine_handle<RunTaskPromise>;

  struct FinalAwaiter {
    bool await_ready() noexcept {
      return false;
    }

    void await_suspend(handle coroutine) const noexcept {
      coroutine.promise().event_->Complete();
    }

    void await_resume() noexcept {
    }
  };

 public:
  RunTaskPromise() = default;

  RunTask<T> get_return_object() noexcept {
    return RunTask<T>(handle::from_promise(*this));
  }

  std::suspend_always initial_suspend() noexcept {
    return {};
  }

  void return_value(auto arg) {
    result_.SetValue(std::move(arg));
  }

  void unhandled_exception() noexcept {
    result_.SetException(std::current_exception());
  }

  auto final_suspend() noexcept {
    return FinalAwaiter{};
  }

 public:
  void Start(threads::MPSCEvent& event) {
    event_ = &event;
    handle::from_promise(*this).resume();
  }

  T UnwrapResult() {
    return result_.Unwrap();
  }

 private:
  threads::MPSCEvent* event_{nullptr};
  result::Result<T> result_;
};

}  // namespace harmony::coro::impl
