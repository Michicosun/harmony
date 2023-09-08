#pragma once

#include <harmony/coro/combine/impl/task_group/group_shared_state.hpp>
#include <harmony/coro/core/base_promise.hpp>

namespace harmony::coro::impl {

class GroupTask;

class GroupTaskPromise : public BasePromise {
  using handle = std::coroutine_handle<GroupTaskPromise>;

  struct FinalAwaiter {
    bool await_ready() noexcept {
      return false;
    }

    void await_suspend(handle coroutine) const noexcept {
      GroupTaskPromise& promise = coroutine.promise();
      promise.shared_state_->CompleteOne();
      coroutine.destroy();
    }

    void await_resume() noexcept {
    }
  };

 public:
  GroupTaskPromise() = default;

  GroupTask get_return_object() noexcept;
  std::suspend_always initial_suspend() noexcept;
  void return_void();
  void unhandled_exception() noexcept;
  FinalAwaiter final_suspend() noexcept;

 public:
  void Start(GroupSharedState* shared_state);

 private:
  GroupSharedState* shared_state_{nullptr};
};

}  // namespace harmony::coro::impl
