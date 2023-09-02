#pragma once

#include <atomic>
#include <coroutine>
#include <exception>
#include <stop_token>

#include <harmony/coro/combine/impl/rendezvous.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/result/result.hpp>
#include <harmony/support/ref_counter/ref_counter.hpp>

namespace harmony::coro::impl {

template <class T>
class AllTask;

class AllSharedStateBase : public support::RefCounter {
 public:
  explicit AllSharedStateBase(size_t tasks_cnt) {
    running_cnt_.fetch_add(tasks_cnt + 1);  // +1 waiter
    IncRefCount(tasks_cnt + 1);             // +1 waiter
  }

  virtual ~AllSharedStateBase() = default;

  void Register(std::coroutine_handle<> waiter) {
    waiter_ = waiter;
  }

 public:
  void CompleteOne() {
    size_t prev_running_cnt = running_cnt_.fetch_sub(1);

    if (prev_running_cnt == 2) {  // last is waiter
      if (consensus_.CompleteResult()) {
        waiter_.resume();
      }
    }

    DecRefCount();
  }

  void Interrupt(std::exception_ptr ptr) {
    if (!has_saved_error_.exchange(true)) {
      e_ptr_ = ptr;
      stop_source_.request_stop();

      if (consensus_.CompleteResult()) {
        waiter_.resume();
      }
    }
  }

 protected:
  // sync logic
  Rendezvous consensus_;
  std::atomic<size_t> running_cnt_{0};
  std::atomic<bool> has_saved_error_{false};

  // state
  std::coroutine_handle<> waiter_{nullptr};
  std::exception_ptr e_ptr_{nullptr};

  // cancellation
  std::stop_source stop_source_;
};

}  // namespace harmony::coro::impl
