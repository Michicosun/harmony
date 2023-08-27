#pragma once

#include <utility>

#include <harmony/runtime/executors/concept.hpp>
#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::runtime {

struct IScheduler {
  virtual void Schedule(
      executors::TaskBase* task,
      executors::ExecutorHint hint = executors::ExecutorHint::Unspecified) = 0;
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

  Scheduler& WithTimer() {
    return *this;
  }

  void Start() {
    executor_.Start();
  }

  void Stop() {
    executor_.Stop();
  }

 private:
  Executor executor_;
  // io source
  // timer source
};

}  // namespace harmony::runtime
