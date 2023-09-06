#pragma once

#include <harmony/coro/events/impl/fd_awaiter.hpp>

namespace harmony::coro {

inline auto FdReady(io::Fd fd, io::Operation operation) noexcept {
  return impl::FdAwaiter(fd, operation);
}

}  // namespace harmony::coro
