#include <harmony/coro/combine/impl/task_group/add_task_awaiter.hpp>
#include <harmony/coro/combine/impl/task_group/group_shared_state.hpp>

namespace harmony::coro::impl {

GroupSharedState::GroupSharedState() {
  running_cnt_.fetch_add(1);  // for last wait
  IncRefCount(1);             // for last wait
}

bool GroupSharedState::Register(std::coroutine_handle<> waiter,
                                std::stop_token stop_token) {
  waiter_ = waiter;
  stop_callback_.emplace(stop_token, UpstreamStopHandler{this});

  if (consensus_.DoneStartingTasks()) {
    stop_callback_.reset();
    return false;  // should not suspend
  }

  return true;
}

AddTaskAwaiter GroupSharedState::AddTask(GroupTask&& task) {
  return AddTaskAwaiter(this, std::move(task));
}

void GroupSharedState::CompleteOne() {
  size_t prev_running_cnt = running_cnt_.fetch_sub(1);

  if (prev_running_cnt == 2) {  // last is waiter
    if (consensus_.CompleteResult()) {
      stop_callback_.reset();  // remove upstream stop handler
      waiter_.resume();
    }
  }

  DecRefCount();
}

void GroupSharedState::CancelAll() {
  stop_source_.request_stop();
}

}  // namespace harmony::coro::impl
