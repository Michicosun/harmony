#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::coro::impl {

class YieldAwaiter : public executors::TaskBase {
 public:
  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  void await_suspend(std::coroutine_handle<Promise> coroutine) noexcept {
    BasePromise& promise = coroutine.promise();

    // extract current scheduler and check it
    auto coro_parameters = promise.GetParameters();
    coro_parameters.CheckActiveScheduler();

    // schedule awake
    coroutine_ = coroutine;
    coro_parameters.scheduler_->Schedule(this);
  }

  void await_resume() noexcept;

 public:
  void Run() noexcept override;

 private:
  std::coroutine_handle<> coroutine_{nullptr};
};

}  // namespace harmony::coro::impl
