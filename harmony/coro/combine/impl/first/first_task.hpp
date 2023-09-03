#pragma once

#include <harmony/coro/combine/impl/first/first_task_promise.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/coro/traits/awaitable.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

template <class T>
class FirstTask {
 public:
  using promise_type = FirstTaskPromise<T>;

  FirstTask(FirstTask&& t) noexcept
      : coro_(std::exchange(t.coro_, {})) {
  }

  ~FirstTask() {
    if (coro_ && coro_.done()) {
      coro_.destroy();
    } else if (coro_ && !coro_.done()) {
      support::Terminate("first_task destroyed before coroutine was completed");
    }
  }

 public:
  void Start(FirstSharedStateBase<T>* shared_state) {
    coro_.promise().Start(shared_state);
  }

  promise_type& GetPromise() {
    return coro_.promise();
  }

 public:
  void SetParameters(runtime::IScheduler* scheduler,
                     std::stop_token stop_token) {
    FirstTaskPromise<T>& promise = GetPromise();
    auto& parameters = promise.GetParameters();

    // setup parameters for wrapper task to push them inside the actual tasks
    parameters.scheduler = scheduler;
    parameters.stop_token = stop_token;
  }

 private:
  friend class FirstTaskPromise<T>;

  explicit FirstTask(std::coroutine_handle<promise_type> h) noexcept
      : coro_(h) {
  }

 private:
  std::coroutine_handle<promise_type> coro_;
};

template <concepts::Awaitable Awaitable,
          class ReturnType = traits::AwaitableTraits<Awaitable>::AwaiterReturnT>
static FirstTask<ReturnType> CreateFirstTask(Awaitable&& object) {
  auto saved_object = std::move(object);
  co_return co_await saved_object;
}

}  // namespace harmony::coro::impl
