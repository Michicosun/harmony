#pragma once

#include <stop_token>

#include <harmony/coro/combine/impl/rendezvous.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/support/ref_counter/ref_counter.hpp>
#include <harmony/support/result/result.hpp>

namespace harmony::coro::impl {

template <class T>
class FirstTask;

template <class T>
class FirstSharedStateBase : public support::RefCounter {
  struct UpstreamStopHandler {
    FirstSharedStateBase* shared_state{nullptr};

    explicit UpstreamStopHandler(FirstSharedStateBase* shared_state)
        : shared_state{shared_state} {
    }

    void operator()() {
      shared_state->stop_source_.request_stop();
    }
  };

 public:
  explicit FirstSharedStateBase(size_t tasks_cnt) {
    running_cnt_.fetch_add(tasks_cnt + 1);  // +1 waiter
    IncRefCount(tasks_cnt + 1);             // +1 waiter
  }

  virtual ~FirstSharedStateBase() = default;

  void Register(std::coroutine_handle<> waiter, std::stop_token stop_token) {
    waiter_ = waiter;
    stop_callback_.emplace(stop_token, UpstreamStopHandler{this});
  }

 public:
  void SetValue(T value) {
    running_cnt_.fetch_sub(1);

    if (!has_result_.exchange(true)) {
      result_.SetValue(std::move(value));
      stop_source_.request_stop();

      if (consensus_.CompleteResult()) {
        stop_callback_.reset();  // remove upstream stop handler
        waiter_.resume();
      }
    }
  }

  void SetException(std::exception_ptr ptr) {
    size_t prev_running_cnt = running_cnt_.fetch_sub(1);

    if (prev_running_cnt > 2) {  // not last task
      return;
    }

    if (!has_result_.exchange(true)) {
      result_.SetException(ptr);

      if (consensus_.CompleteResult()) {
        stop_callback_.reset();  // remove upstream stop handler
        waiter_.resume();
      }
    }
  }

  void CompleteOne() {
    DecRefCount();
  }

 protected:
  // sync logic
  Rendezvous consensus_;
  std::atomic<size_t> running_cnt_{0};
  std::atomic<bool> has_result_{false};

  // state
  std::coroutine_handle<> waiter_{nullptr};
  support::Result<T> result_;

  // cancellation
  std::stop_source stop_source_;
  std::optional<std::stop_callback<UpstreamStopHandler>> stop_callback_;
};

}  // namespace harmony::coro::impl
