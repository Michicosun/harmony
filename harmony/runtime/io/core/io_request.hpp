#pragma once

#include <sys/epoll.h>

#include <harmony/runtime/event.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>
#include <harmony/support/intrusive/forward_list.hpp>

namespace harmony::io {

struct IORequest : public runtime::Event,
                   public support::ForwardListNode<IORequest> {
  Fd fd;
  io::Operation operation;

  uint64_t id;
  epoll_event ev;

  EventStatus status;
};

}  // namespace harmony::io
