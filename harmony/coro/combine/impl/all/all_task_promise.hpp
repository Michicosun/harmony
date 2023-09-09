#pragma once

#include <harmony/coro/combine/impl/all/all_shared_state_base.hpp>
#include <harmony/coro/core/base_promise.hpp>
#include <harmony/result/result.hpp>

namespace harmony::coro::impl {

template <class T>
class AllTask;

template <class T>
class AllTaskPromise : public BasePromise {
  using handle = std::coroutine_handle<AllTaskPromise>;

  struct FinalAwaiter {
    bool await_ready() noexcept {
      return false;
    }

    void await_suspend(handle coroutine) const noexcept {
      AllTaskPromise& promise = coroutine.promise();

      if (promise.result_.HasError()) {
        promise.shared_state_->Interrupt(promise.result_.Error());
      }

      promise.shared_state_->CompleteOne();
    }

    void await_resume() noexcept {
    }
  };

 public:
  AllTaskPromise() = default;

  AllTask<T> get_return_object() noexcept {
    return AllTask<T>(handle::from_promise(*this));
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
  void Start(AllSharedStateBase* shared_state) {
    shared_state_ = shared_state;
    handle::from_promise(*this).resume();
  }

  T UnwrapResult() {
    return result_.Unwrap();
  }

 private:
  AllSharedStateBase* shared_state_{nullptr};
  result::Result<T> result_;
};

}  // namespace harmony::coro::impl
