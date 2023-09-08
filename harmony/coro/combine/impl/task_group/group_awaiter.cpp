#include <harmony/coro/combine/impl/task_group/group_awaiter.hpp>

namespace harmony::coro::impl {

GroupAwaiter::GroupAwaiter(GroupSharedState* shared_state)
    : shared_state_{shared_state} {
}

GroupAwaiter::~GroupAwaiter() {
  shared_state_->CancelAll();
  shared_state_->CompleteOne();
}

bool GroupAwaiter::await_ready() noexcept {
  return false;
}

void GroupAwaiter::await_resume() {
  CheckCancel(parameters_);
}

}  // namespace harmony::coro::impl
