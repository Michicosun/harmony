#pragma once

#include <cstddef>

#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/interface.hpp>
#include <harmony/runtime/executors/task.hpp>
#include <harmony/support/intrusive/forward_list.hpp>

namespace harmony::executors {

class ManualExecutor : public IExecutor {
 public:
  ManualExecutor();

  void Submit(TaskBase* task,
              ExecutorHint = ExecutorHint::Unspecified) override;

  ~ManualExecutor() override = default;

  bool RunNext();
  size_t Run(size_t limit);
  size_t Drain();

  size_t TasksInQueue() const;
  bool IsEmpty() const;

 private:
  void RunOne();

 private:
  support::ForwardList<TaskBase> tasks_;
};

}  // namespace harmony::executors
