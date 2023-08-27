#pragma once

#include <harmony/support/intrusive/forward_list.hpp>

namespace harmony::executors {

struct ITask {
  virtual ~ITask() = default;
  virtual void Run() noexcept = 0;
};

struct TaskBase : ITask,
                  support::ForwardListNode<TaskBase> {
  //
};

}  // namespace harmony::executors
