#pragma once

#include <sys/epoll.h>
#include <cstdint>
#include <stdexcept>

namespace harmony::io {

class EpollError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

enum class Operation {
  Read,
  Write,
};

enum class EventStatus {
  Ready,
  Error,
  Closed,
};

EventStatus EventToStatus(uint32_t event_set);
uint32_t OperationToEvent(Operation op);

}  // namespace harmony::io
