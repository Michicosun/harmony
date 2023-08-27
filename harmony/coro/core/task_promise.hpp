#pragma once

#include <cassert>
#include <coroutine>
#include <exception>
#include <utility>

#include <harmony/result/result.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro {

template <class T>
class Task;

class ThisCoroType {};
constexpr ThisCoroType kThisCoro;

struct CoroParameters {
  runtime::IScheduler* scheduler_{nullptr};

  void CheckActiveScheduler() const {
    assert(scheduler_);
  }
};

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

  struct CoroParametersAwaiter {
    constexpr bool await_ready() {
      return true;
    }

    void await_suspend(handle) {
    }

    auto await_resume() noexcept {
      return promise->GetParameters();
    }

    TaskPromise* promise;
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
    return CoroParametersAwaiter{this};
  }

  decltype(auto) await_transform(auto&& awaiter) {
    return std::forward<decltype(awaiter)>(awaiter);
  }

 public:
  CoroParameters& GetParameters() {
    return parameters_;
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
  friend struct CoroParametersAwaiter;

  result::Result<T> result_;
  std::coroutine_handle<> continuation_ = std::noop_coroutine();
  CoroParameters parameters_;
};

}  // namespace harmony::coro
