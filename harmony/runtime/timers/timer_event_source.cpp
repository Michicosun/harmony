#include <harmony/runtime/timers/timer_event_source.hpp>

namespace harmony::timers {

void TimerEventSource::Start() {
  worker_ = std::thread([this]() {
    WorkerRoutine();
  });
}

void TimerEventSource::WaitIdle() {
  running_timers_.Wait();
}

void TimerEventSource::Stop() {
  stopped_.store(true);
  worker_.join();
}

void TimerEventSource::AddTimer(TimerBase* timer, Deadline deadline) {
  timer->id = next_free_id_.fetch_add(1);
  timer->deadline = deadline;

  running_timers_.Add(1);
  new_timers_.Push(timer);
}

void TimerEventSource::AddTimer(TimerBase* timer, Duration timeout) {
  AddTimer(timer, DeadlineFromNow(timeout));
}

void TimerEventSource::DeleteTimer(TimerBase* timer) {
  timers_to_cancel_.Push(timer);
}

void TimerEventSource::WorkerRoutine() {
  while (!stopped_.load()) {
    AddNewTimers();
    DeleteCancelledTimers();
    ProcessTimers();
    SuspendWorker();
  }
}

void TimerEventSource::AddNewTimers() {
  TimerBase* timer = new_timers_.ExtractAll();

  while (timer != nullptr) {
    timers_.push({timer, timer->id});
    timer = Unwrap(timer->next);
  }
}

void TimerEventSource::DeleteCancelledTimers() {
  TimerBase* timer = timers_to_cancel_.ExtractAll();

  while (timer != nullptr) {
    deleted_timers_.insert(timer->id);
    timer->OnFinish();
    timer = Unwrap(timer->next);
    running_timers_.Done();
  }
}

void TimerEventSource::ProcessTimers() {
  auto now = Clock::now();
  size_t processed = 0;

  while (!timers_.empty() && processed < kMaxBatchSize) {
    const auto& timer_handle = timers_.top();
    TimerBase* timer = timer_handle.timer;
    uint64_t id = timer_handle.id;

    if (auto it = deleted_timers_.find(id); it != deleted_timers_.end()) {
      deleted_timers_.erase(it);  // timer is already deleted -> skipping
      timers_.pop();
      continue;
    }

    if (timer->deadline > now) {
      break;
    }

    if (timer->state.Finish()) {
      timer->OnFinish();
      running_timers_.Done();
      ++processed;
    }

    timers_.pop();
  }
}

void TimerEventSource::SuspendWorker() const {
  std::this_thread::sleep_for(kSleepTime);
}

}  // namespace harmony::timers
