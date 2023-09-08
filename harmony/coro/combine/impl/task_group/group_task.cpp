#include <harmony/coro/combine/impl/task_group/group_promise.hpp>
#include <harmony/coro/combine/impl/task_group/group_task.hpp>

namespace harmony::coro::impl {

GroupTask::GroupTask(GroupTask&& t) noexcept
    : coro_(std::exchange(t.coro_, {})) {
}

void GroupTask::Start(GroupSharedState* shared_state) {
  coro_.promise().Start(shared_state);
}

GroupTask::promise_type& GroupTask::GetPromise() {
  return coro_.promise();
}

void GroupTask::SetParameters(runtime::IScheduler* scheduler,
                              std::stop_token stop_token) {
  GroupTaskPromise& promise = GetPromise();
  auto& parameters = promise.GetParameters();

  // setup parameters for wrapper task to push them inside the actual tasks
  parameters.scheduler = scheduler;
  parameters.stop_token = stop_token;
}

GroupTask::GroupTask(std::coroutine_handle<promise_type> h) noexcept
    : coro_(h) {
}

}  // namespace harmony::coro::impl
