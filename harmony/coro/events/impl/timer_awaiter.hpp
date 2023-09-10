#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/scheduler.hpp>
#include <harmony/runtime/timers/core/deadline.hpp>
#include <harmony/runtime/timers/core/timer.hpp>
#include <harmony/threads/event/event.hpp>

namespace harmony::coro::impl {

class [[nodiscard]] TimerAwaiter : public executors::TaskBase,
                                   public timers::TimerBase {
  struct TimerStopHandler {
    TimerAwaiter* timer{nullptr};

    explicit TimerStopHandler(TimerAwaiter* timer)
        : timer{timer} {
    }

    void operator()() {
      if (timer->state.Cancel()) {
        auto& ts = timer->parameters_->scheduler->GetTimerEventSource();
        ts.DeleteTimer(timer);
      }
    }
  };

 public:
  explicit TimerAwaiter(timers::Duration duration);

  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  void await_suspend(std::coroutine_handle<Promise> coroutine) {
    BasePromise& promise = coroutine.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // check scheduler
    parameters_->CheckActiveScheduler();

    // save coro handle for resume
    coroutine_ = coroutine;

    // setup timer and stop callback
    auto& ts = parameters_->scheduler->GetTimerEventSource();
    ts.AddTimer(this, duration_);

    stop_callback_.emplace(parameters_->stop_token, TimerStopHandler(this));
    cb_constructed_.store(true);
  }

  void await_resume();

 public:
  void Schedule();
  void Run() noexcept override;

 public:
  void OnFinish() override;

 private:
  timers::Duration duration_;

  std::coroutine_handle<> coroutine_{nullptr};
  CoroParameters* parameters_{nullptr};

  std::atomic<bool> cb_constructed_{false};
  std::optional<std::stop_callback<TimerStopHandler>> stop_callback_;
};

}  // namespace harmony::coro::impl
