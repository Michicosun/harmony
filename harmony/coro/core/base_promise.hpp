#pragma once

#include <cassert>
#include <coroutine>
#include <stop_token>

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro {

class ThisCoroType {};
constexpr ThisCoroType kThisCoro;

struct CoroParameters {
  runtime::IScheduler* scheduler_{nullptr};
  std::stop_token stop_token_;

  void CheckActiveScheduler() const {
    assert(scheduler_);
  }

  void MergeFrom(const CoroParameters& other) {
    scheduler_ = other.scheduler_;
    stop_token_ = other.stop_token_;
  }
};

class BasePromise {
  struct CoroParametersAwaiter {
    constexpr bool await_ready() {
      return true;
    }

    void await_suspend(std::coroutine_handle<>) {
    }

    auto await_resume() noexcept {
      return promise->GetParameters();
    }

    BasePromise* promise;
  };

 public:
  template <concepts::BaseTask Task>
  Task& await_transform(Task& other_task) {
    BasePromise& other_promise = other_task.GetPromise();

    // push parameters to child task
    other_promise.GetParameters().MergeFrom(parameters_);

    return other_task;
  }

  template <concepts::BaseTask Task>
  Task await_transform(Task&& other_task) {
    BasePromise& other_promise = other_task.GetPromise();

    // push parameters to child task
    other_promise.GetParameters().MergeFrom(parameters_);

    return std::move(other_task);
  }

  auto await_transform(const ThisCoroType&) {
    return CoroParametersAwaiter{this};
  }

  decltype(auto) await_transform(auto&& awaiter) {
    return std::forward<decltype(awaiter)>(awaiter);
  }

 public:
  CoroParameters& GetParameters() {
    return parameters_;
  }

 private:
  friend struct CoroParametersAwaiter;

  CoroParameters parameters_;
};

}  // namespace harmony::coro
