#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>

#include <tl/expected.hpp>

namespace harmony::coro {

template <class T>
class Task;

template <class T>
class TaskPromise {
  struct FinalAwaiter {
    bool await_ready() noexcept {
      return false;
    }

    auto await_suspend(std::coroutine_handle<TaskPromise> h) noexcept {
      return h.promise().ReleaseContinuation();
    }

    void await_resume() noexcept {
    }
  };

 public:
  Task<T> get_return_object() noexcept {
    return Task<T>{std::coroutine_handle<TaskPromise<T>>::from_promise(*this)};
  }

  // TODO: change to lazy initialization
  std::suspend_never initial_suspend() noexcept {
    return {};
  }

  void return_value(auto arg) {
    result_ = tl::expected<T, std::exception_ptr>{std::move(arg)};
  }

  void unhandled_exception() noexcept {
    result_ = tl::unexpected<std::exception_ptr>(std::current_exception());
  }

 public:
  FinalAwaiter final_suspend() noexcept {
    return {};
  }

  void SetContinuation(std::coroutine_handle<> continuation) {
    continuation_ = std::move(continuation);
  }

  std::coroutine_handle<> ReleaseContinuation() {
    return std::move(continuation_);
  }

  bool HasResult() const {
    return result_.has_value();
  }

  T UnwrapResult() {
    if (!result_.has_value()) {
      std::terminate();  // impossible
    }

    tl::expected<T, std::exception_ptr>& expected = result_.value();

    if (expected.has_value()) {
      return std::move(expected.value());
    }

    std::rethrow_exception(expected.error());
  }

 private:
  std::optional<tl::expected<T, std::exception_ptr>> result_;
  std::coroutine_handle<> continuation_ = std::noop_coroutine();
};

}  // namespace harmony::coro
