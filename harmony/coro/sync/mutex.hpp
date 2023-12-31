#pragma once

#include <harmony/coro/concepts/base_task.hpp>
#include <harmony/coro/core/task_promise.hpp>
#include <harmony/runtime/executors/task.hpp>
#include <harmony/runtime/scheduler.hpp>
#include <harmony/support/intrusive/forward_list.hpp>
#include <harmony/support/intrusive/node_unwrap.hpp>

namespace harmony::coro {

class Mutex {
 public:
  enum class UnlockStrategy {
    Schedule,
    Resume,
  };

 private:
  struct UniqueLock {
   public:
    explicit UniqueLock(Mutex* mutex, UnlockStrategy strategy)
        : mutex{mutex},
          strategy(strategy) {
    }

    ~UniqueLock() {
      mutex->Unlock(strategy);
    }

    Mutex* mutex{nullptr};
    UnlockStrategy strategy;
  };

  struct ScheduleTask : public executors::TaskBase {
    void Start(UnlockStrategy strategy) {
      if (strategy == UnlockStrategy::Schedule) {
        Schedule();
      } else {
        Run();
      }
    }

    std::coroutine_handle<> suspended_coro{nullptr};
    runtime::IScheduler* scheduler{nullptr};

   private:
    void Schedule() {
      scheduler->Schedule(this);
    }

    void Run() noexcept {
      suspended_coro.resume();
    }
  };

  struct [[nodiscard]] LockAwaiter
      : public support::ForwardListNode<LockAwaiter> {
    explicit LockAwaiter(Mutex* mutex, UnlockStrategy strategy) noexcept
        : mutex{mutex},
          strategy(strategy) {
      assert(mutex);
    }

    bool await_ready() const noexcept {
      return mutex->TryLock();
    }

    template <concepts::BasePromiseConvertible Promise>
    bool await_suspend(std::coroutine_handle<Promise> awaiter) {
      BasePromise& promise = awaiter.promise();
      parameters_ = &promise.GetParameters();

      // check cancel request
      CheckCancel(parameters_);

      // save scheduler
      parameters_->CheckActiveScheduler();
      schedule_task.scheduler = parameters_->scheduler;

      // save coro handler
      schedule_task.suspended_coro = awaiter;

      return mutex->LockOrPark(this) != LockResult::Acquired;
    }

    UniqueLock await_resume() const {
      UniqueLock unlocker{mutex, strategy};
      CheckCancel(parameters_);
      return unlocker;
    }

    Mutex* mutex{nullptr};
    CoroParameters* parameters_{nullptr};
    ScheduleTask schedule_task;
    UnlockStrategy strategy;
  };

 public:
  inline auto ScopedLock(
      UnlockStrategy strategy = UnlockStrategy::Resume) noexcept {
    return LockAwaiter{this, strategy};
  }

 private:
  bool TryLock() noexcept {
    auto old_state = Unlocked;
    return state_.compare_exchange_strong(old_state, LockedNoWaiters);
  }

  void Unlock(UnlockStrategy strategy) {
    assert(state_.load() != Unlocked);

    if (!waiters_list_.IsEmpty()) {
      LockAwaiter* head = waiters_list_.PopFront();
      head->schedule_task.Start(strategy);
    } else {
      auto old_state = LockedNoWaiters;
      if (state_.compare_exchange_strong(old_state, Unlocked)) {
        return;
      }

      old_state = state_.exchange(LockedNoWaiters);
      assert(old_state != LockedNoWaiters && old_state != Unlocked);

      LockAwaiter* tail = reinterpret_cast<LockAwaiter*>(old_state);
      PushReversed(tail);

      assert(!waiters_list_.IsEmpty());

      LockAwaiter* head = waiters_list_.PopFront();
      head->schedule_task.Start(strategy);
    }
  }

 private:
  enum State : uintptr_t {
    LockedNoWaiters = 0,
    Unlocked = 1,
    // pointer to waiters intrusive queue
  };

  enum class LockResult {
    Acquired,
    Parked,
  };

 private:
  void PushReversed(LockAwaiter* head) {
    assert(waiters_list_.IsEmpty());

    while (head != nullptr) {
      waiters_list_.PushFront(std::exchange(head, support::Unwrap(head->next)));
    }
  }

  LockResult LockOrPark(LockAwaiter* awaiter) {
    auto old_state = state_.load();

    while (true) {
      if (old_state == Unlocked) {
        if (state_.compare_exchange_weak(old_state, LockedNoWaiters)) {
          return LockResult::Acquired;
        }
      } else {
        awaiter->next = reinterpret_cast<LockAwaiter*>(old_state);
        auto updated = static_cast<State>(reinterpret_cast<uintptr_t>(awaiter));
        if (state_.compare_exchange_weak(old_state, updated)) {
          return LockResult::Parked;
        }
      }
    }
  }

 private:
  std::atomic<State> state_{Unlocked};
  support::ForwardList<LockAwaiter> waiters_list_;
};

}  // namespace harmony::coro
