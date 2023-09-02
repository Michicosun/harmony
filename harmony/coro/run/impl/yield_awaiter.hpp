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
    auto& parameters = promise.GetParameters();

    // extract current scheduler and check it
    parameters.CheckActiveScheduler();

    // schedule awake
    coroutine_ = coroutine;
    parameters.scheduler->Schedule(this);
  }

  void await_resume() noexcept;

 public:
  void Run() noexcept override;

 private:
  std::coroutine_handle<> coroutine_{nullptr};
};

}  // namespace harmony::coro::impl
