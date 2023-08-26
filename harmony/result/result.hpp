#pragma once

#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>

namespace harmony::result {

class MyException {};

template <class T>
class Result {
 public:
  Result() = default;

  Result(Result&& other)
      : value_(std::exchange(other.value_, std::nullopt)),
        e_ptr_(std::exchange(other.e_ptr_, nullptr)) {
  }

  void SetException(std::exception_ptr e_ptr) noexcept {
    e_ptr_ = e_ptr;
  }

  void SetValue(T&& value) {
    value_.emplace(std::forward<T>(value));
  }

  T Unwrap() {
    if (!value_ && !e_ptr_) {
      std::terminate();
    }

    if (value_.has_value()) {
      return std::move(value_.value());
    }

    std::rethrow_exception(e_ptr_);
  }

 private:
  std::optional<T> value_{};
  std::exception_ptr e_ptr_{nullptr};
};

}  // namespace harmony::result
