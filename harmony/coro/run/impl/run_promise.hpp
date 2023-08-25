#pragma once

#include <coroutine>
#include <optional>

#include <tl/expected.hpp>

#include <harmony/support/event.hpp>

namespace harmony::coro {

template <class T>
class RunTask;

template <class T>
class RunTaskPromise {
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
    result_ = tl::expected<T, std::exception_ptr>{std::move(arg)};
  }

  void unhandled_exception() noexcept {
    result_ = tl::unexpected<std::exception_ptr>(std::current_exception());
  }

  auto final_suspend() noexcept {
    return FinalAwaiter{};
  }

 public:
  void Start(support::MPSCEvent& event) {
    event_ = &event;
    handle::from_promise(*this).resume();
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
  support::MPSCEvent* event_{nullptr};
  std::optional<tl::expected<T, std::exception_ptr>> result_;
};

}  // namespace harmony::coro
