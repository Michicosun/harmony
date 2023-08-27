#pragma once

#include <coroutine>
#include <exception>
#include <utility>

#include <harmony/executors/interface.hpp>
#include <harmony/result/result.hpp>

namespace harmony::coro {

template <class T>
class Task;

class ThisCoroType {};
constexpr ThisCoroType kThisCoro;

struct ThisCoroParameters {};

template <class T>
class TaskPromise {
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

  struct ThisCoroParameterAwaiter {
    constexpr bool await_ready() {
      return true;
    }

    void await_suspend(std::coroutine_handle<>) {
    }

    auto await_resume() noexcept {
      return ThisCoroParameters{};
    }

    executors::IExecutor* executor;
  };

 public:
  Task<T> get_return_object() noexcept {
    return Task<T>(handle::from_promise(*this));
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

  FinalAwaiter final_suspend() noexcept {
    return {};
  }

  auto await_transform(const ThisCoroType&) {
    return ThisCoroParameterAwaiter{};
  }

  decltype(auto) await_transform(auto&& awaiter) {
    return std::forward<decltype(awaiter)>(awaiter);
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
