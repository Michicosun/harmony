#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::coro::impl {

class YieldAwaiter : public executors::TaskBase {
 public:
  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  void await_suspend(std::coroutine_handle<Promise> coroutine) {
    BasePromise& promise = coroutine.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // extract current scheduler and check it
    parameters_->CheckActiveScheduler();

    // schedule awake
    coroutine_ = coroutine;
    parameters_->scheduler->Schedule(this);
  }

  void await_resume();

 public:
  void Run() noexcept override;

 private:
  std::coroutine_handle<> coroutine_{nullptr};
  CoroParameters* parameters_{nullptr};
};

}  // namespace harmony::coro::impl
