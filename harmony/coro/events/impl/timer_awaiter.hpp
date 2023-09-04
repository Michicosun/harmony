#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/scheduler.hpp>
#include <harmony/runtime/timers/core/deadline.hpp>
#include <harmony/runtime/timers/core/timer.hpp>
#include <harmony/support/event/event.hpp>

namespace harmony::coro::impl {

class [[nodiscard]] TimerAwaiter : public executors::TaskBase,
                                   public timers::TimerBase {
  struct TimerStopHandler {
    TimerAwaiter* timer{nullptr};
    uint64_t timer_id{0};

    TimerStopHandler(TimerAwaiter* timer, uint64_t timer_id)
        : timer{timer},
          timer_id{timer_id} {
    }

    void operator()() {
      if (timer->state.Cancel()) {
        auto& ts = timer->parameters_->scheduler->GetTimerEventSource();
        ts.DeleteTimer(timer_id);
        timer->Schedule();
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
    uint64_t timer_id = ts.AddTimer(this, duration_);

    stop_callback_.emplace(parameters_->stop_token,
                           TimerStopHandler(this, timer_id));
    cb_constructed_.Complete();
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

  support::MPSCEvent cb_constructed_;
  std::optional<std::stop_callback<TimerStopHandler>> stop_callback_;
};

}  // namespace harmony::coro::impl
