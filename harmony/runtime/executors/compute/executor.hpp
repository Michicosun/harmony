#pragma once

#include <thread>
#include <vector>

#include <harmony/runtime/executors/hint.hpp>
#include <harmony/runtime/executors/interface.hpp>
#include <harmony/runtime/executors/task.hpp>
#include <harmony/support/queues/blocking_queue.hpp>
#include <harmony/support/wait_group/wait_group.hpp>

namespace harmony::executors {

class ComputeExecutor : public IExecutor {
 public:
  explicit ComputeExecutor(size_t threads);
  ~ComputeExecutor();

  void Submit(TaskBase* task,
              ExecutorHint = ExecutorHint::Unspecified) override;

  void Start();
  void Stop();
  void WaitIdle();

 private:
  void WorkerRoutine();

 private:
  size_t threads_count_;
  std::vector<std::thread> workers_;
  support::WaitGroup planned_tasks_;
  support::UnboundedBlockingQueue<TaskBase> queue_;
};

}  // namespace harmony::executors
