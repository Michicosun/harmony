#pragma once

#include <concepts>
#include <type_traits>

namespace harmony::coro {

class BasePromise;

namespace concepts {

template <class T>
concept BaseTask = requires(T type) {
  { type.GetPromise() } -> std::convertible_to<BasePromise&>;
};

}  // namespace concepts

}  // namespace harmony::coro
