#pragma once

#include <harmony/coro/combine/impl/all/all_shared_state.hpp>
#include <harmony/coro/combine/impl/all/all_task.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro::impl {

template <class... Results>
class AllAwaiter {
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

    // register waiter for wake up
    shared_state_->Register(waiter);

    // start tasks inside waiter scheduler
    auto& parameters = promise.GetParameters();
    shared_state_->StartTasks(parameters.scheduler_);
  }

  std::tuple<Results...> await_resume() {
    return shared_state_->Unwrap();
  }

 private:
  AllSharedState<Results...>* shared_state_{nullptr};
};

}  // namespace harmony::coro::impl
