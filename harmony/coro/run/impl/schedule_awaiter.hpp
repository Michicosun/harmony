#pragma once

#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

class ScheduleAwaiter : public executors::TaskBase {
 public:
  explicit ScheduleAwaiter(runtime::IScheduler* scheduler);

  bool await_ready() noexcept;

  template <class T>
  void await_suspend(std::coroutine_handle<TaskPromise<T>> coroutine) noexcept {
    TaskPromise<T>& promise = coroutine.promise();

    // set new scheduler to coro
    promise.GetParameters().scheduler_ = scheduler_;

    // schedule awake
    coroutine_ = coroutine;
    scheduler_->Schedule(this);
  }

  void await_resume() noexcept;

 public:
  void Run() noexcept override;

 private:
  runtime::IScheduler* scheduler_{nullptr};
  std::coroutine_handle<> coroutine_{nullptr};
};

}  // namespace harmony::coro::impl
