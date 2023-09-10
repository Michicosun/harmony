#pragma once

#include <sys/epoll.h>
#include <cstdint>

namespace harmony::io {

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
