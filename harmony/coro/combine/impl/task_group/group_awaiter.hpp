#pragma once

#include <harmony/coro/combine/impl/task_group/group_shared_state.hpp>
#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/core/base_promise.hpp>

namespace harmony::coro::impl {

class [[nodiscard]] GroupAwaiter {
 public:
  explicit GroupAwaiter(GroupSharedState* shared_state);

  ~GroupAwaiter();

 public:
  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  bool await_suspend(std::coroutine_handle<Promise> waiter) {
    BasePromise& promise = waiter.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // register waiter for wake up
    return shared_state_->Register(waiter, parameters_->stop_token);
  }

  void await_resume();

 private:
  GroupSharedState* shared_state_{nullptr};
  CoroParameters* parameters_{nullptr};
};

}  // namespace harmony::coro::impl
