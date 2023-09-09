#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>

#include <harmony/runtime/timers/core/deadline.hpp>
#include <harmony/runtime/timers/core/timer.hpp>
#include <harmony/threads/spin_lock/spin_lock.hpp>

namespace harmony::timers {

struct TimerHandle {
  TimerBase* timer{nullptr};
  Deadline deadline;
  uint64_t id;

  bool operator<(const TimerHandle& th) const {
    return deadline > th.deadline;
  }
};

class TimerEventSource {
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

  uint64_t AddTimer(TimerBase* timer, Deadline deadline) {
    std::lock_guard guard(spin_lock_);

    uint64_t id = next_free_id_++;
    running_.insert(id);

    timers_.push(TimerHandle{
        .timer = timer,
        .deadline = deadline,
        .id = id,
    });

    return id;
  }

  uint64_t AddTimer(TimerBase* timer, Duration timeout) {
    return AddTimer(timer, DeadlineFromNow(timeout));
  }

  void DeleteTimer(uint64_t id) {
    std::lock_guard guard(spin_lock_);
    running_.erase(id);
  }

 private:
  void WorkerRoutine() {
    while (!stopped_.load()) {
      GrabTimersBatch();
      AlarmBatch();
      SuspendWorker();
    }
  }

  void GrabTimersBatch() {
    std::lock_guard guard(spin_lock_);

    auto now = Clock::now();
    batch_size_ = 0;

    while (!timers_.empty() && batch_size_ < kMaxBatchSize) {
      const auto& timer_handle = timers_.top();

      if (timer_handle.deadline > now) {
        break;
      }

      uint64_t timer_id = timer_handle.id;
      auto it = running_.find(timer_id);

      if (it != running_.end()) {
        timers_batch_[batch_size_++] = timer_handle.timer;
        running_.erase(it);
      }

      timers_.pop();
    }
  }

  void AlarmBatch() {
    for (size_t i = 0; i < batch_size_; ++i) {
      TimerBase* timer_event = timers_batch_[i];

      if (timer_event->state.Finish()) {
        timer_event->OnFinish();
      }
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

  threads::SpinLock spin_lock_;
  std::priority_queue<TimerHandle> timers_;
  std::unordered_set<uint64_t> running_;
  uint64_t next_free_id_{0};

  size_t batch_size_{0};
  std::array<TimerBase*, kMaxBatchSize> timers_batch_;
};

};  // namespace harmony::timers
