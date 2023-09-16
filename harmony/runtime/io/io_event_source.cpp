#include <sys/epoll.h>
#include <cstring>

#include <harmony/runtime/io/io_event_source.hpp>

namespace harmony::io {

IOEventSource::IOEventSource() {
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

void IOEventSource::Start() {
  worker_ = std::thread([this]() {
    WorkerRoutine();
  });
}

void IOEventSource::WaitIdle() {
  running_requests_.Wait();
}

void IOEventSource::Stop() {
  stopped_.store(true);
  worker_.join();
}

void IOEventSource::AddIORequest(IORequest* request) {
  request->id = next_free_id_.fetch_add(1);
  request->ev.data.ptr = request;
  request->ev.events =
      OperationToEvent(request->operation) | EPOLLONESHOT | EPOLLRDHUP;

  running_requests_.Add(1);
  new_requests_.Push(request);
  has_new_requests_.Signal();
}

void IOEventSource::DeleteIORequest(IORequest* request) {
  requests_to_cancel_.Push(request);
  has_cancellation_.Signal();
}

void IOEventSource::WorkerRoutine() {
  while (!stopped_.load()) {
    AddNewRequests();
    DeleteCancelledRequests();
    GrabIOEventsBatch();
    ProcessEvents();
  }
}

void IOEventSource::GrabIOEventsBatch() {
  batch_size_ =
      epoll_wait(epoll_fd_, events_.data(), kMaxBatchSize, kWaitTimeout);
}

void IOEventSource::AddNewRequests() {
  if (!has_new_requests_.Reset()) {
    return;
  }

  IORequest* request = new_requests_.ExtractAll();

  while (request != nullptr) {
    IORequest* next = Unwrap(request->next);
    SubmitIORequestToEpoll(request);
    request = next;
  }
}

void IOEventSource::DeleteCancelledRequests() {
  if (!has_cancellation_.Reset()) {
    return;
  }

  IORequest* request = requests_to_cancel_.ExtractAll();

  while (request != nullptr) {
    IORequest* next = Unwrap(request->next);

    RemoveIORequestFromEpoll(request);
    request->OnFinish();
    running_requests_.Done();

    request = next;
  }
}

void IOEventSource::ProcessEvents() {
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
      running_requests_.Done();
    }
  }
}

void IOEventSource::SubmitIORequestToEpoll(IORequest* request) {
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, request->fd, &request->ev);
}

void IOEventSource::RemoveIORequestFromEpoll(IORequest* request) {
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, request->fd, nullptr);
}

}  // namespace harmony::io
