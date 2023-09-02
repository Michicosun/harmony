#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

class ScheduleAwaiter : public executors::TaskBase {
 public:
  explicit ScheduleAwaiter(runtime::IScheduler* scheduler);

  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  void await_suspend(std::coroutine_handle<Promise> coroutine) {
    BasePromise& promise = coroutine.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // set new scheduler to coro
    parameters_->scheduler = scheduler_;

    // schedule awake
    coroutine_ = coroutine;
    scheduler_->Schedule(this);
  }

  void await_resume();

 public:
  void Run() noexcept override;

 private:
  runtime::IScheduler* scheduler_{nullptr};
  std::coroutine_handle<> coroutine_{nullptr};
  CoroParameters* parameters_{nullptr};
};

}  // namespace harmony::coro::impl
