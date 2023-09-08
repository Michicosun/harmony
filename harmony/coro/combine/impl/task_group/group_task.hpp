#pragma once

#include <harmony/coro/combine/impl/task_group/group_promise.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

class GroupTask {
 public:
  using promise_type = GroupTaskPromise;

  GroupTask(GroupTask&& t) noexcept;

 public:
  void Start(GroupSharedState* shared_state);

  promise_type& GetPromise();

 public:
  void SetParameters(runtime::IScheduler* scheduler,
                     std::stop_token stop_token);

 private:
  friend class GroupTaskPromise;

  explicit GroupTask(std::coroutine_handle<promise_type> h) noexcept;

 private:
  std::coroutine_handle<promise_type> coro_;
};

template <concepts::Awaitable Awaitable>
static GroupTask CreateGroupTask(Awaitable&& object) {
  auto saved_object = std::move(object);
  co_await saved_object;
}

}  // namespace harmony::coro::impl
