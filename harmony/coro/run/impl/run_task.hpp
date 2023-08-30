#pragma once

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/run/impl/run_task_promise.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro::impl {

template <class T>
class RunTask {
 public:
  using promise_type = RunTaskPromise<T>;

  ~RunTask() {
    if (coro_ && !coro_.done()) {
      support::Terminate("task was destroyed before coroutine was completed");
    }

    if (coro_ && coro_.done()) {
      coro_.destroy();
    }
  }

  void Start(support::MPSCEvent& event) {
    coro_.promise().Start(event);
  }

  T UnwrapResult() {
    return coro_.promise().UnwrapResult();
  }

 private:
  friend class RunTaskPromise<T>;

  explicit RunTask(std::coroutine_handle<promise_type> h) noexcept
      : coro_(h) {
  }

 private:
  std::coroutine_handle<promise_type> coro_;
};

template <concepts::Awaitable Awaitable,
          class ReturnType = traits::AwaitableTraits<Awaitable>::AwaiterReturnT>
static RunTask<ReturnType> CreateRunTask(Awaitable&& object) {
  auto saved_object = std::move(object);
  co_return co_await saved_object;
}

}  // namespace harmony::coro::impl
