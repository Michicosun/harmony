#pragma once

#include <atomic>
#include <coroutine>

#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/core/base_promise.hpp>
#include <harmony/support/intrusive/forward_list.hpp>
#include <harmony/support/intrusive/node_unwrap.hpp>
#include <harmony/support/queues/closable_lock_free_queue.hpp>

namespace harmony::coro {

class WaitGroup {
  struct [[nodiscard]] WaitAwaiter
      : public support::ForwardListNode<WaitAwaiter> {
    explicit WaitAwaiter(WaitGroup* wait_group)
        : wait_group{wait_group} {
    }

    bool await_ready() const noexcept {
      return wait_group->balance_.load() == 0;
    }

    template <concepts::BasePromiseConvertible Promise>
    bool await_suspend(std::coroutine_handle<Promise> coroutine) {
      BasePromise& promise = coroutine.promise();
      parameters_ = &promise.GetParameters();

      // check cancel request
      CheckCancel(parameters_);

      stopped_coroutine = coroutine;
      return wait_group->awaiters_.Push(this);
    }

    void await_resume() {
      CheckCancel(parameters_);
    }

    std::coroutine_handle<> stopped_coroutine{nullptr};
    WaitGroup* wait_group{nullptr};
    CoroParameters* parameters_{nullptr};
  };

 public:
  void Add(size_t count) {
    balance_.fetch_add(count);
  }

  void Done() {
    if (balance_.fetch_sub(1) == 1) {
      WaitAwaiter* awaiter = awaiters_.Close();

      while (awaiter != nullptr) {
        WaitAwaiter* next = support::Unwrap(awaiter->Next());
        awaiter->stopped_coroutine.resume();
        awaiter = next;
      }
    }
  }

  inline auto Wait() {
    return WaitAwaiter{this};
  }

 private:
  std::atomic<size_t> balance_{0};
  support::ClosableLockFreeQueue<WaitAwaiter> awaiters_;
};

}  // namespace harmony::coro
