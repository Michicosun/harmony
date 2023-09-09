#pragma once

#include <harmony/coro/combine/impl/first/first_shared_state.hpp>
#include <harmony/coro/core/base_promise.hpp>
#include <harmony/support/result/result.hpp>

namespace harmony::coro::impl {

template <class T>
class FirstTask;

template <class T>
class FirstTaskPromise : public BasePromise {
  using handle = std::coroutine_handle<FirstTaskPromise>;

  struct FinalAwaiter {
    bool await_ready() noexcept {
      return false;
    }

    void await_suspend(handle coroutine) const noexcept {
      FirstTaskPromise& promise = coroutine.promise();

      if (promise.result_.HasError()) {
        promise.shared_state_->SetException(promise.result_.Error());
      } else {
        promise.shared_state_->SetValue(promise.result_.Value());
      }

      promise.shared_state_->CompleteOne();
    }

    void await_resume() noexcept {
    }
  };

 public:
  FirstTaskPromise() = default;

  FirstTask<T> get_return_object() noexcept {
    return FirstTask<T>(handle::from_promise(*this));
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
  void Start(FirstSharedStateBase<T>* shared_state) {
    shared_state_ = shared_state;
    handle::from_promise(*this).resume();
  }

 private:
  FirstSharedStateBase<T>* shared_state_{nullptr};
  support::Result<T> result_;
};

}  // namespace harmony::coro::impl
