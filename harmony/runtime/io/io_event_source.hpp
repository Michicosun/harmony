#pragma once

#include <sys/epoll.h>
#include <array>
#include <cstring>
#include <thread>

#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/event_fd.hpp>
#include <harmony/runtime/io/core/exceptions.hpp>
#include <harmony/runtime/io/core/fd.hpp>
#include <harmony/runtime/io/core/io_request.hpp>
#include <harmony/support/queues/batch_lock_free_queue.hpp>
#include <harmony/threads/spin_lock/spin_lock.hpp>

namespace harmony::io {

class IOEventSource {
 public:
  IOEventSource() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
      throw EpollError(strerror(errno));
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = this;

    io::Fd new_fd = has_new_requests_.NativeHandle();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, new_fd, &ev) == -1) {
      throw EpollError(strerror(errno));
    }

    io::Fd cancel_fd = has_cancellation_.NativeHandle();
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, cancel_fd, &ev) == -1) {
      throw EpollError(strerror(errno));
    }
  }

  void Start() {
    worker_ = std::thread([this]() {
      WorkerRoutine();
    });
  }

  void Stop() {
    stopped_.store(true);
    worker_.join();
  }

  void AddIORequest(IORequest* request) {
    request->id = next_free_id_.fetch_add(1);
    request->ev.data.ptr = request;
    request->ev.events =
        OperationToEvent(request->operation) | EPOLLONESHOT | EPOLLRDHUP;

    new_requests_.Push(request);
    has_new_requests_.Signal();
  }

  void DeleteIORequest(IORequest* request) {
    requests_to_cancel_.Push(request);
    has_cancellation_.Signal();
  }

 private:
  void WorkerRoutine() {
    while (!stopped_.load()) {
      GrabIOEventsBatch();
      AddNewRequests();
      DeleteCancelledRequests();
      ProcessEvents();
    }
  }

  void GrabIOEventsBatch() {
    batch_size_ =
        epoll_wait(epoll_fd_, events_.data(), kMaxBatchSize, kWaitTimeout);
  }

  void AddNewRequests() {
    if (!has_new_requests_.Reset()) {
      return;
    }

    IORequest* request = new_requests_.ExtractAll();

    while (request != nullptr) {
      SubmitIORequestToEpoll(request);
      request = Unwrap(request->next);
    }
  }

  void DeleteCancelledRequests() {
    if (!has_cancellation_.Reset()) {
      return;
    }

    IORequest* request = requests_to_cancel_.ExtractAll();

    while (request != nullptr) {
      RemoveIORequestFromEpoll(request);
      request->OnFinish();
      request = Unwrap(request->next);
    }
  }

  void ProcessEvents() {
    for (int32_t i = 0; i < batch_size_; ++i) {
      const auto& event = events_[i];

      if (event.data.ptr == this) {
        // internal signal -> skipping
        continue;
      }

      IORequest* request = static_cast<IORequest*>(event.data.ptr);

      if (request->state.Finish()) {
        RemoveIORequestFromEpoll(request);
        request->status = EventToStatus(event.events);
        request->OnFinish();
      }
    }
  }

 private:
  void SubmitIORequestToEpoll(IORequest* request) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, request->fd, &request->ev);
  }

  void RemoveIORequestFromEpoll(IORequest* request) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, request->fd, nullptr);
  }

 private:
  static constexpr size_t kMaxBatchSize = 1 << 12;
  static constexpr size_t kWaitTimeout = 1000;  // 1s

  Fd epoll_fd_{-1};

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
