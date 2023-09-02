#pragma once

#include <harmony/coro/concepts/base_task.hpp>

namespace harmony::coro::concepts {

template <class T>
concept BasePromiseConvertible = std::convertible_to<T, BasePromise>;

}  // namespace harmony::coro::concepts
