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
      auto continuation = h.promise().ReleaseContinuation();

      if (h.promise().OwnsLifetime()) {
        h.destroy();
      }

      return continuation;
    }

    void await_resume() noexcept {
    }
  };

 public:
  Task<T> get_return_object() noexcept {
    return Task<T>{std::coroutine_handle<TaskPromise<T>>::from_promise(*this)};
  }

  std::suspend_always initial_suspend() noexcept {
    return {};
  }

  void return_value(auto arg) {
    result_ = tl::expected<T, std::exception_ptr>{std::move(arg)};
  }

  void unhandled_exception() noexcept {
    result_ = tl::unexpected<std::exception_ptr>(std::current_exception());
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
    if (!result_.has_value()) {
      std::terminate();  // impossible
    }

    tl::expected<T, std::exception_ptr>& expected = result_.value();

    if (expected.has_value()) {
      return std::move(expected.value());
    }

    std::rethrow_exception(expected.error());
  }

  void TransferLifetime() {
    owns_lifetime_ = true;
  }

  bool OwnsLifetime() const {
    return owns_lifetime_;
  }

 private:
  friend class Task<T>;

  std::optional<tl::expected<T, std::exception_ptr>> result_;
  std::coroutine_handle<> continuation_ = std::noop_coroutine();
  bool owns_lifetime_{false};
};

}  // namespace harmony::coro
