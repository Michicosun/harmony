#pragma once

#include <sys/epoll.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>
#include <harmony/runtime/io/core/io_request.hpp>
#include <harmony/threads/spin_lock.hpp>

namespace harmony::io {

struct IORequestHandle {
  IORequest* request;
  epoll_event ev;

  IORequestHandle(IORequest* request, Operation op, uint64_t id)
      : request{request} {
    ev.events = OperationToEvent(op) | EPOLLONESHOT | EPOLLRDHUP;
    ev.data.u64 = id;
  }
};

class IOEventSource {
 public:
  IOEventSource()
      : epoll_fd_(epoll_create1(EPOLL_CLOEXEC)) {
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

  uint64_t AddIORequest(IORequest* request, Operation op) {
    std::lock_guard guard(spin_lock_);

    uint64_t id = next_free_id_++;
    auto it = requests_.insert({id, IORequestHandle(request, op, id)});

    auto& io_handle = it.first->second;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, request->fd, &io_handle.ev) == -1) {
      throw EpollError("error while adding fd to epoll");
    }

    return id;
  }

  void DeleteIORequest(Fd fd, uint64_t id) {
    std::lock_guard guard(spin_lock_);
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    requests_.erase(id);
  }

 private:
  void WorkerRoutine() {
    while (!stopped_.load()) {
      GrabIOEventsBatch();
      ProcessEvents();
    }
  }

  void GrabIOEventsBatch() {
    batch_size_ =
        epoll_wait(epoll_fd_, events_.data(), kMaxBatchSize, kWaitTimeout);
  }

  void ProcessEvents() {
    std::lock_guard guard(spin_lock_);

    for (int32_t i = 0; i < batch_size_; ++i) {
      const auto& event = events_[i];
      uint64_t request_id = event.data.u64;

      // check if request already cancelled
      auto it = requests_.find(request_id);
      if (it == requests_.end()) {
        continue;
      }

      IORequest* request = it->second.request;

      if (request->fd != -1) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, request->fd, nullptr);
      }

      request->status = EventToStatus(event.events);

      if (request->state.Finish()) {
        request->OnFinish();
      }
    }
  }

 private:
  static constexpr size_t kMaxBatchSize = 1 << 12;
  static constexpr size_t kWaitTimeout = 1000;  // 1s

  Fd epoll_fd_{-1};

  std::thread worker_;
  std::atomic<bool> stopped_{false};

  threads::SpinLock spin_lock_;
  std::unordered_map<uint64_t, IORequestHandle> requests_;
  uint64_t next_free_id_{0};

  int32_t batch_size_{0};
  std::array<epoll_event, kMaxBatchSize> events_;
};

};  // namespace harmony::io
