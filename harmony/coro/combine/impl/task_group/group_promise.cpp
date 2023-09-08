#include <harmony/coro/combine/impl/task_group/group_promise.hpp>
#include <harmony/coro/combine/impl/task_group/group_task.hpp>

namespace harmony::coro::impl {

GroupTask GroupTaskPromise::get_return_object() noexcept {
  return GroupTask(handle::from_promise(*this));
}

std::suspend_always GroupTaskPromise::initial_suspend() noexcept {
  return {};
}

void GroupTaskPromise::return_void() {
}

void GroupTaskPromise::unhandled_exception() noexcept {
}

GroupTaskPromise::FinalAwaiter GroupTaskPromise::final_suspend() noexcept {
  return {};
}

void GroupTaskPromise::Start(GroupSharedState* shared_state) {
  shared_state_ = shared_state;
  handle::from_promise(*this).resume();
}

}  // namespace harmony::coro::impl
