#pragma once

#include <sys/epoll.h>

#include <harmony/runtime/event.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::io {

struct IORequest : public runtime::Event {
  Fd fd;
  io::Operation operation;
  EventStatus status;
};

}  // namespace harmony::io
