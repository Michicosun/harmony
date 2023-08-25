#pragma once

#include <harmony/coro/concepts/awaiter.hpp>
#include <harmony/coro/concepts/awaitable.hpp>

namespace harmony::coro::concepts {

template <class T, class ReturnType>
concept PromiseReturn = requires(T type, ReturnType return_type) {
  requires std::same_as<decltype(type.return_void()), void> ||
               std::same_as<decltype(type.return_value(return_type)), void> ||
               requires { type.yield_value(return_type); };
};

template <class T, class ReturnType>
concept Promise = requires(T type) {
  { type.initial_suspend() } -> Awaiter;
  { type.final_suspend() } -> Awaiter;
  { type.yield_value() } -> Awaitable;
} && PromiseReturn<T, ReturnType>;

}  // namespace harmony::coro::concepts
