#include <harmony/coro/combine/task_group.hpp>

namespace harmony::coro {

TaskGroup::TaskGroup() {
  shared_state_ = new impl::GroupSharedState();
}

impl::GroupAwaiter TaskGroup::Wait() const {
  return impl::GroupAwaiter(shared_state_);
}

}  // namespace harmony::coro
