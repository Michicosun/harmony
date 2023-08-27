#pragma once

#include <coroutine>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::coro::impl {

class YieldAwaiter : public executors::TaskBase {
 public:
  bool await_ready() noexcept;

  template <class T>
  void await_suspend(std::coroutine_handle<TaskPromise<T>> coroutine) noexcept {
    TaskPromise<T>& promise = coroutine.promise();

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
  std::coroutine_handle<> coroutine_;
};

}  // namespace harmony::coro::impl
