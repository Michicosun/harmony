#pragma once

#include <harmony/coro/run/impl/yield_awaiter.hpp>

namespace harmony::coro {

inline auto Yield() {
  return impl::YieldAwaiter();
}

}  // namespace harmony::coro
