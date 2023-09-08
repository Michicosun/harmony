#pragma once

#include <harmony/coro/combine/impl/task_group/add_task_awaiter.hpp>
#include <harmony/coro/combine/impl/task_group/group_awaiter.hpp>
#include <harmony/coro/concepts/awaitable.hpp>

namespace harmony::coro {

class TaskGroup {
 public:
  TaskGroup();

  template <concepts::Awaitable Task>
  auto Start(Task&& task) {
    auto wrapped_task = impl::CreateGroupTask(std::move(task));
    return shared_state_->AddTask(std::move(wrapped_task));
  }

  impl::GroupAwaiter Wait() const;

 private:
  impl::GroupSharedState* shared_state_{nullptr};
};

}  // namespace harmony::coro
