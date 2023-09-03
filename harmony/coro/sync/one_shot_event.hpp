#pragma once

#include <coroutine>

#include <harmony/coro/concepts/base_promise.hpp>
#include <harmony/coro/core/base_promise.hpp>
#include <harmony/support/intrusive/forward_list.hpp>
#include <harmony/support/intrusive/node_unwrap.hpp>
#include <harmony/support/queues/closable_lock_free_queue.hpp>

namespace harmony::coro {

class OneShotEvent {
  struct [[nodiscard]] ReadyAwaiter
      : public support::ForwardListNode<ReadyAwaiter> {
    explicit ReadyAwaiter(OneShotEvent* event)
        : event{event} {
    }

    bool await_ready() const noexcept {
      return event->awaiters_.IsClosed();
    }

    template <concepts::BasePromiseConvertible Promise>
    bool await_suspend(std::coroutine_handle<Promise> coroutine) {
      BasePromise& promise = coroutine.promise();
      parameters_ = &promise.GetParameters();

      // check cancel request
      CheckCancel(parameters_);

      stopped_coroutine = coroutine;
      return event->awaiters_.Push(this);
    }

    void await_resume() {
      CheckCancel(parameters_);
    }

    std::coroutine_handle<> stopped_coroutine{nullptr};
    OneShotEvent* event{nullptr};
    CoroParameters* parameters_{nullptr};
  };

 public:
  inline auto Wait() {
    return ReadyAwaiter{this};
  }

  void Complete() {
    ReadyAwaiter* awaiter = awaiters_.Close();

    while (awaiter != nullptr) {
      ReadyAwaiter* next = support::Unwrap(awaiter->Next());
      awaiter->stopped_coroutine.resume();
      awaiter = next;
    }
  }

 private:
  friend struct ReadyAwaiter;

  support::ClosableLockFreeQueue<ReadyAwaiter> awaiters_;
};

}  // namespace harmony::coro
