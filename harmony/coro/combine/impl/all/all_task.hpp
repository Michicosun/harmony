#pragma once

#include <harmony/coro/combine/impl/all/all_shared_state_base.hpp>
#include <harmony/coro/combine/impl/all/all_task_promise.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

template <class T>
class AllTask {
 public:
  using promise_type = AllTaskPromise<T>;

  AllTask(AllTask&& t) noexcept
      : coro_(std::exchange(t.coro_, {})) {
  }

  ~AllTask() {
    if (coro_ && !coro_.done()) {
      support::Terminate(
          "all_task was destroyed before coroutine was completed");
    }

    if (coro_ && coro_.done()) {
      coro_.destroy();
    }
  }

 public:
  void Start(AllSharedStateBase* shared_state) {
    coro_.promise().Start(shared_state);
  }

  T UnwrapResult() {
    return coro_.promise().UnwrapResult();
  }

  promise_type& GetPromise() {
    return coro_.promise();
  }

 public:
  void SetParameters(runtime::IScheduler* scheduler,
                     std::stop_token stop_token) {
    AllTaskPromise<T>& promise = GetPromise();

    // setup parameters for wrapper task to push them inside the actual tasks
    promise.GetParameters().scheduler_ = scheduler;
    promise.GetParameters().stop_token_ = stop_token;
  }

 private:
  friend class AllTaskPromise<T>;

  explicit AllTask(std::coroutine_handle<promise_type> h) noexcept
      : coro_(h) {
  }

 private:
  std::coroutine_handle<promise_type> coro_;
};

template <concepts::Awaitable Awaitable,
          class ReturnType = traits::AwaitableTraits<Awaitable>::AwaiterReturnT>
static AllTask<ReturnType> CreateAllTask(Awaitable&& object) {
  auto saved_object = std::move(object);
  co_return co_await saved_object;
}

}  // namespace harmony::coro::impl
