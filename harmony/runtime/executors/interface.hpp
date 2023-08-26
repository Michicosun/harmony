#pragma once

#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/task.hpp>

namespace harmony::runtime::executors {

struct IExecutor {
  virtual ~IExecutor() = default;
  virtual void Submit(TaskBase* task,
                      ExecutorHint = ExecutorHint::Unspecified) = 0;
};

}  // namespace harmony::runtime::executors
