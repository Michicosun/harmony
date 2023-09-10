#include <sys/eventfd.h>
#include <cerrno>
#include <cstring>

#include <harmony/runtime/io/core/event_fd.hpp>
#include <harmony/runtime/io/core/exceptions.hpp>

namespace harmony::io {

EventFd::EventFd() {
  efd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

  if (efd_ < 0) {
    throw EventFdError(strerror(errno));
  }
}

void EventFd::Signal() {
  if (!signalled_.exchange(true)) {
    eventfd_t signal = 1;
    eventfd_write(efd_, signal);
  }
}

bool EventFd::Reset() {
  if (signalled_.load()) {
    eventfd_t signal = 0;
    eventfd_read(efd_, &signal);

    signalled_.exchange(false);

    return true;
  }

  return false;
}

io::Fd EventFd::NativeHandle() const {
  return efd_;
}

}  // namespace harmony::io
