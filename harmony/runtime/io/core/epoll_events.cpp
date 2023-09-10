#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/exceptions.hpp>

namespace harmony::io {

namespace {

bool Intersects(uint32_t event_set, uint32_t events) {
  return (event_set & events) != 0;
}

}  // namespace

EventStatus EventToStatus(uint32_t event_set) {
  if (Intersects(event_set, EPOLLPRI | EPOLLERR)) {
    return EventStatus::Error;
  }

  if (Intersects(event_set, EPOLLRDHUP | EPOLLHUP)) {
    return EventStatus::Closed;
  }

  if (Intersects(event_set, EPOLLIN | EPOLLOUT)) {
    return EventStatus::Ready;
  }

  throw EpollError("unexpected epoll event status");
}

uint32_t OperationToEvent(Operation op) {
  switch (op) {
    case Operation::Read:
      return EPOLLIN;
    case Operation::Write:
      return EPOLLOUT;
  }
}

}  // namespace harmony::io
