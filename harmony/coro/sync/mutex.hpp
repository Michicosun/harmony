#pragma once

#include <atomic>
#include <coroutine>

#include <harmony/support/intrusive/forward_list.hpp>
#include <utility>
#include "harmony/coro/core/task_promise.hpp"
#include "harmony/runtime/executors/task.hpp"
#include "harmony/runtime/scheduler.hpp"

namespace harmony::coro {

class Mutex {
  struct UniqueLock {
    explicit UniqueLock(Mutex& mutex)
        : mutex_{mutex} {
    }

    ~UniqueLock() {
      mutex_.Unlock();
    }

   private:
    Mutex& mutex_;
  };

  struct LockAwaiter : executors::TaskBase {
    explicit LockAwaiter(Mutex& mutex)
        : mutex_{mutex} {
    }

    bool await_ready() const noexcept {
      return mutex_.TryLock();
    }

    template <class T>
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<TaskPromise<T>> coroutine) {
      TaskPromise<T>& promise = coroutine.promise();

      // save waiting coroutine scheduler
      promise.GetParameters().CheckActiveScheduler();
      scheduler_ = promise.GetParameters().scheduler_;

      // save coroutine
      waiting_coro_ = coroutine;

      if (mutex_.LockOrPark(this) == AwaitStatus::Resume) {
        return coroutine;
      }

      return std::noop_coroutine();
    }

    UniqueLock await_resume() {
      return UniqueLock(mutex_);
    }

    void Schedule() noexcept {
      scheduler_->Schedule(this);
    }

    void Run() noexcept {
      waiting_coro_.resume();
    }

    LockAwaiter* next{nullptr};

   private:
    Mutex& mutex_;
    runtime::IScheduler* scheduler_{nullptr};
    std::coroutine_handle<> waiting_coro_{nullptr};
  };

 public:
  inline auto ScopedLock() noexcept {
    return LockAwaiter{*this};
  }

  bool TryLock() {
    auto unlock_state = MutexState::Unlocked;
    return state_.compare_exchange_strong(unlock_state,
                                          MutexState::LockedEmptyPushQueue);
  }

  void Unlock() {
    if (waiters_head_ != nullptr) {
      ExtractHead()->Schedule();
    } else {
      auto cur_state = state_.load();

      while (true) {
        if (cur_state == MutexState::LockedEmptyPushQueue) {
          if (state_.compare_exchange_weak(cur_state, MutexState::Unlocked)) {
            return;
          }
        } else {
          LockAwaiter* awaiters = CastToLockAwaiter(
              state_.exchange(MutexState::LockedEmptyPushQueue));

          waiters_head_ = Reverse(awaiters);
          ExtractHead()->Schedule();

          return;
        }
      }
    }
  }

 private:
  enum MutexState : uintptr_t {
    Unlocked = 0,
    LockedEmptyPushQueue = 1,
    // LockAwaiter* ptr
  };

  enum class AwaitStatus {
    Resume,
    Suspend,
  };

 private:
  LockAwaiter* CastToLockAwaiter(MutexState state) {
    return reinterpret_cast<LockAwaiter*>(state);
  }

  MutexState CastToMutexState(LockAwaiter* awaiter) {
    uintptr_t ptr = (uintptr_t)(void*)awaiter;
    return (MutexState)ptr;
  }

  AwaitStatus LockOrPark(LockAwaiter* awaiter) {
    auto cur_state = state_.load();

    while (true) {
      if (cur_state == MutexState::Unlocked) {
        if (state_.compare_exchange_weak(cur_state,
                                         MutexState::LockedEmptyPushQueue)) {
          return AwaitStatus::Resume;
        }
      } else {
        if (cur_state == MutexState::LockedEmptyPushQueue) {
          awaiter->next = nullptr;
        } else {
          awaiter->next = CastToLockAwaiter(cur_state);
        }

        if (state_.compare_exchange_weak(cur_state,
                                         CastToMutexState(awaiter))) {
          return AwaitStatus::Suspend;
        }
      }
    }
  }

 private:
  LockAwaiter* ExtractHead() {
    return std::exchange(waiters_head_, waiters_head_->next);
  }

  LockAwaiter* Reverse(LockAwaiter* cur) {
    LockAwaiter* prev = nullptr;

    while (cur != nullptr) {
      LockAwaiter* next = cur->next;
      cur->next = prev;
      prev = cur;
      cur = next;
    }

    return prev;
  }

 private:
  std::atomic<MutexState> state_{MutexState::Unlocked};
  LockAwaiter* waiters_head_{nullptr};
};

}  // namespace harmony::coro
