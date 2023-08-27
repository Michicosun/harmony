#pragma once

#include <coroutine>

#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

class ScheduleAwaiter : public executors::TaskBase {
 public:
  explicit ScheduleAwaiter(runtime::IScheduler* scheduler);

  bool await_ready() noexcept;

  template <class T>
  void await_suspend(std::coroutine_handle<TaskPromise<T>> coroutine) noexcept {
    coroutine_ = coroutine;
    coroutine.promise().SetScheduler(scheduler_);
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
