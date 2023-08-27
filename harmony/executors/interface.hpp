#pragma once

#include <harmony/executors/hint.hpp>
#include <harmony/executors/task.hpp>

namespace harmony::executors {

struct IExecutor {
  virtual ~IExecutor() = default;
  virtual void Submit(TaskBase* task,
                      ExecutorHint = ExecutorHint::Unspecified) = 0;
};

}  // namespace harmony::executors
