#pragma once

#include <harmony/coro/combine/impl/all/all_shared_state.hpp>
#include <harmony/coro/combine/impl/all/all_task.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro::impl {

template <class... Results>
class [[nodiscard]] AllAwaiter {
 public:
  explicit AllAwaiter(AllTask<Results>&&... tasks) {
    shared_state_ =
        new AllSharedState(std::forward<AllTask<Results>>(tasks)...);
  }

  ~AllAwaiter() {
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

  std::tuple<Results...> await_resume() {
    CheckCancel(parameters_);
    return shared_state_->Unwrap();
  }

 private:
  AllSharedState<Results...>* shared_state_{nullptr};
  CoroParameters* parameters_{nullptr};
};

}  // namespace harmony::coro::impl
