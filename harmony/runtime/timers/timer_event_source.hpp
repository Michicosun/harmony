#pragma once

#include <atomic>
#include <cstddef>
#include <queue>
#include <thread>

#include <harmony/runtime/timers/core/deadline.hpp>
#include <harmony/runtime/timers/core/timer.hpp>
#include <harmony/support/queues/batch_lock_free_queue.hpp>
#include <harmony/threads/spin_lock/spin_lock.hpp>

namespace harmony::timers {

class TimerEventSource {
  struct TimerHandle {
    TimerBase* timer{nullptr};

    bool operator<(const TimerHandle& th) const {
      return timer->deadline > th.timer->deadline;
    }
  };

 public:
  void Start() {
    worker_ = std::thread([this]() {
      WorkerRoutine();
    });
  }

  void Stop() {
    stopped_.store(true);
    worker_.join();
  }

  void AddTimer(TimerBase* timer, Deadline deadline) {
    timer->id = next_free_id_.fetch_add(1);
    timer->deadline = deadline;
    new_timers_.Push(timer);
  }

  void AddTimer(TimerBase* timer, Duration timeout) {
    AddTimer(timer, DeadlineFromNow(timeout));
  }

  void DeleteTimer(TimerBase* timer) {
    timers_to_cancel_.Push(timer);
  }

 private:
  void WorkerRoutine() {
    while (!stopped_.load()) {
      AddNewTimers();
      DeleteCancelledTimers();
      ProcessTimers();
      SuspendWorker();
    }
  }

  void AddNewTimers() {
    TimerBase* timer = new_timers_.ExtractAll();

    while (timer != nullptr) {
      timers_.push({timer});
      timer = Unwrap(timer->next);
    }
  }

  void DeleteCancelledTimers() {
    TimerBase* timer = timers_to_cancel_.ExtractAll();

    while (timer != nullptr) {
      timer->OnFinish();
      timer = Unwrap(timer->next);
    }
  }

  void ProcessTimers() {
    auto now = Clock::now();
    size_t processed = 0;

    while (!timers_.empty() && processed < kMaxBatchSize) {
      const auto& timer_handle = timers_.top();
      TimerBase* timer = timer_handle.timer;

      if (timer->deadline > now) {
        break;
      }

      if (timer->state.Finish()) {
        timer->OnFinish();
        ++processed;
      }

      timers_.pop();
    }
  }

  void SuspendWorker() const {
    std::this_thread::sleep_for(kSleepTime);
  }

 private:
  static constexpr size_t kMaxBatchSize = 1 << 12;
  static constexpr std::chrono::milliseconds kSleepTime = 1ms;

  std::thread worker_;
  std::atomic<bool> stopped_{false};

  support::BatchLockFreeQueue<TimerBase> new_timers_;
  support::BatchLockFreeQueue<TimerBase> timers_to_cancel_;

  std::priority_queue<TimerHandle> timers_;
  std::atomic<uint64_t> next_free_id_{0};
};

};  // namespace harmony::timers
