#pragma once

#include <harmony/coro/concepts/awaiter.hpp>

namespace harmony::coro::concepts {

template <class T>
concept Awaitable = requires(T type) {
  { type.operator co_await() } -> Awaiter;
};

}
