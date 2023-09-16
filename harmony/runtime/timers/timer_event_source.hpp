#pragma once

#include <queue>
#include <thread>
#include <unordered_set>

#include <harmony/runtime/timers/core/deadline.hpp>
#include <harmony/runtime/timers/core/timer.hpp>
#include <harmony/support/queues/batch_lock_free_queue.hpp>

namespace harmony::timers {

class TimerEventSource {
  struct TimerHandle {
    TimerBase* timer{nullptr};
    uint64_t id{0};

    bool operator<(const TimerHandle& th) const {
      return timer->deadline > th.timer->deadline;
    }
  };

 public:
  void Start();
  void Stop();

  void AddTimer(TimerBase* timer, Deadline deadline);
  void AddTimer(TimerBase* timer, Duration timeout);

  void DeleteTimer(TimerBase* timer);

 private:
  void WorkerRoutine();

  void AddNewTimers();
  void DeleteCancelledTimers();
  void ProcessTimers();
  void SuspendWorker() const;

 private:
  static constexpr size_t kMaxBatchSize = 1 << 12;
  static constexpr std::chrono::milliseconds kSleepTime = 1ms;

  std::thread worker_;
  std::atomic<bool> stopped_{false};

  support::BatchLockFreeQueue<TimerBase> new_timers_;
  support::BatchLockFreeQueue<TimerBase> timers_to_cancel_;
  std::unordered_set<uint64_t> deleted_timers_;

  std::priority_queue<TimerHandle> timers_;
  std::atomic<uint64_t> next_free_id_{0};
};

};  // namespace harmony::timers
