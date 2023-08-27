#pragma once

#include <coroutine>

#include <harmony/support/intrusive/forward_list.hpp>
#include <harmony/support/intrusive/node_unwrap.hpp>
#include <harmony/support/queues/closable_lock_free_queue.hpp>

namespace harmony::coro {

class OneShotEvent {
  struct ReadyAwaiter : public support::ForwardListNode<ReadyAwaiter> {
    explicit ReadyAwaiter(OneShotEvent* event)
        : event{event} {
    }

    bool await_ready() const noexcept {
      return event->awaiters_.IsClosed();
    }

    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<> coroutine) noexcept {
      stopped_coroutine = coroutine;

      if (event->awaiters_.Push(this)) {
        return std::noop_coroutine();
      }

      return coroutine;
    }

    void await_resume() noexcept {
    }

    std::coroutine_handle<> stopped_coroutine{nullptr};
    OneShotEvent* event{nullptr};
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
