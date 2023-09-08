#include <harmony/coro/combine/impl/task_group/add_task_awaiter.hpp>

namespace harmony::coro::impl {

AddTaskAwaiter::AddTaskAwaiter(GroupSharedState* shared_state, GroupTask&& task)
    : task_{std::move(task)},
      shared_state_{shared_state} {
}

bool AddTaskAwaiter::await_ready() noexcept {
  return false;
}

void AddTaskAwaiter::await_resume() {
  CheckCancel(parameters_);
}

}  // namespace harmony::coro::impl
