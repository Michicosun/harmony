#pragma once

#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/run/impl/run_promise.hpp>
#include <harmony/coro/traits/awaitable.hpp>

namespace harmony::coro {

template <class T>
class RunTask {
 public:
  using promise_type = RunTaskPromise<T>;

  ~RunTask() {
    if (coro_ && !coro_.done()) {
      std::terminate();
    }

    if (coro_ && coro_.done()) {
      coro_.destroy();
    }
  }

  void Start(support::MPSCEvent& event) {
    coro_.promise().Start(event);
  }

  T UnwrapResult() noexcept {
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

template <concepts::Awaitable awaitable>
static RunTask<typename traits::AwaitableTraits<awaitable>::AwaiterReturnT>
CreateRunTask(awaitable&& object) {
  co_return co_await object;
}

}  // namespace harmony::coro