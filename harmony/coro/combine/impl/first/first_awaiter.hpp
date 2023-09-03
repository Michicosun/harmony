#pragma once

#include <harmony/coro/combine/impl/first/first_shared_state_base.hpp>
#include <harmony/coro/combine/impl/first/first_task.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro::impl {

template <class T, class... Tasks>
class FirstAwaiter {
 public:
  explicit FirstAwaiter(Tasks&&... tasks) {
    shared_state_ =
        new FirstSharedState<T, Tasks...>(std::forward<FirstTask<T>>(tasks)...);
  }

  ~FirstAwaiter() {
    shared_state_->CompleteOne();
  }

 public:
  bool await_ready() noexcept {
    return false;
  }

  template <concepts::BasePromiseConvertible Promise>
  void await_suspend(std::coroutine_handle<Promise> waiter) {
    BasePromise& promise = waiter.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // register waiter for wake up
    shared_state_->Register(waiter, parameters_->stop_token);

    // start tasks inside waiter scheduler
    shared_state_->StartTasks(parameters_->scheduler);
  }

  T await_resume() {
    CheckCancel(parameters_);
    return shared_state_->Unwrap();
  }

 private:
  FirstSharedState<T, Tasks...>* shared_state_{nullptr};
  CoroParameters* parameters_{nullptr};
};

}  // namespace harmony::coro::impl
