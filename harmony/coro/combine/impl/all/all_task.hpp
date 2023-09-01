#pragma once

#include <harmony/coro/combine/impl/all/all_shared_state_base.hpp>
#include <harmony/coro/combine/impl/all/all_task_promise.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>

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

  void Start(AllSharedStateBase* shared_state) {
    coro_.promise().Start(shared_state);
  }

  T UnwrapResult() {
    return coro_.promise().UnwrapResult();
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
