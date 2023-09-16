#pragma once

#include <thread>

#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/event_fd.hpp>
#include <harmony/runtime/io/core/exceptions.hpp>
#include <harmony/runtime/io/core/fd.hpp>
#include <harmony/runtime/io/core/io_request.hpp>
#include <harmony/support/queues/batch_lock_free_queue.hpp>
#include <harmony/threads/wait_group/wait_group.hpp>

namespace harmony::io {

class IOEventSource {
 public:
  IOEventSource();

  void Start();
  void WaitIdle();
  void Stop();

  void AddIORequest(IORequest* request);
  void DeleteIORequest(IORequest* request);

 private:
  void WorkerRoutine();

  void AddNewRequests();
  void DeleteCancelledRequests();
  void GrabIOEventsBatch();
  void ProcessEvents();

 private:
  void SubmitIORequestToEpoll(IORequest* request);
  void RemoveIORequestFromEpoll(IORequest* request);

 private:
  static constexpr size_t kMaxBatchSize = 1 << 12;
  static constexpr size_t kWaitTimeout = 1000;  // 1s

  Fd epoll_fd_{-1};
  threads::WaitGroup running_requests_;

  EventFd has_new_requests_;
  support::BatchLockFreeQueue<IORequest> new_requests_;

  EventFd has_cancellation_;
  support::BatchLockFreeQueue<IORequest> requests_to_cancel_;

  std::thread worker_;
  std::atomic<bool> stopped_{false};
  std::atomic<uint64_t> next_free_id_{0};

  int32_t batch_size_{0};
  std::array<epoll_event, kMaxBatchSize> events_;
};

};  // namespace harmony::io
