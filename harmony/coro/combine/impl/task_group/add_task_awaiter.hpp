#pragma once

#include <harmony/coro/combine/impl/task_group/group_shared_state.hpp>
#include <harmony/coro/combine/impl/task_group/group_task.hpp>
#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/core/base_promise.hpp>

namespace harmony::coro::impl {

class [[nodiscard]] AddTaskAwaiter {
 public:
  explicit AddTaskAwaiter(GroupSharedState* shared_state, GroupTask&& task);

  bool await_ready() noexcept;

  template <concepts::BasePromiseConvertible Promise>
  auto await_suspend(std::coroutine_handle<Promise> coroutine) {
    BasePromise& promise = coroutine.promise();
    parameters_ = &promise.GetParameters();

    // check cancel request
    CheckCancel(parameters_);

    // increment sync state
    shared_state_->running_cnt_.fetch_add(1);
    shared_state_->IncRefCount(1);

    // start background task
    auto token = shared_state_->stop_source_.get_token();
    task_.SetParameters(parameters_->scheduler, token);
    task_.Start(shared_state_);

    return coroutine;
  }

  void await_resume();

 private:
  GroupTask task_;
  GroupSharedState* shared_state_;
  CoroParameters* parameters_{nullptr};
};

}  // namespace harmony::coro::impl
