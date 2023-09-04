#pragma once

#include <cassert>
#include <optional>
#include <utility>

#include <harmony/runtime/executors/concept.hpp>
#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/task.hpp>
#include <harmony/runtime/timers/timer_event_source.hpp>

namespace harmony::runtime {

struct IScheduler {
  virtual void Schedule(
      executors::TaskBase* task,
      executors::ExecutorHint hint = executors::ExecutorHint::Unspecified) = 0;

  virtual timers::TimerEventSource& GetTimerEventSource() = 0;
};

template <executors::Executor Executor>
class Scheduler : public IScheduler {
 public:
  template <class... Args>
  explicit Scheduler(Args&&... executor_args)
      : executor_(std::forward<Args>(executor_args)...) {
  }

  void Schedule(executors::TaskBase* task,
                executors::ExecutorHint hint) override {
    executor_.Submit(task, hint);
  }

  Scheduler& WithIO() {
    return *this;
  }

  template <class... Args>
  Scheduler& WithTimer(Args&&... args) {
    timer_event_source_.emplace(std::forward<Args>(args)...);
    return *this;
  }

 public:
  Executor& GetExecutor() {
    return executor_;
  }

  timers::TimerEventSource& GetTimerEventSource() override {
    assert(timer_event_source_.has_value());
    return timer_event_source_.value();
  }

 public:
  void Start() {
    executor_.Start();

    if (timer_event_source_.has_value()) {
      timer_event_source_->Start();
    }
  }

  void WaitIdle() {
    executor_.WaitIdle();
  }

  void Stop() {
    executor_.Stop();

    if (timer_event_source_.has_value()) {
      timer_event_source_->Stop();
    }
  }

 private:
  Executor executor_;
  // io source
  std::optional<timers::TimerEventSource> timer_event_source_;
};

}  // namespace harmony::runtime
