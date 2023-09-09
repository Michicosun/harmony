#pragma once

#include <cassert>
#include <exception>
#include <optional>
#include <utility>

#include <harmony/support/terminate/terminate.hpp>

namespace harmony::support {

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

  bool HasValue() const {
    return value_.has_value();
  }

  bool HasError() const {
    return e_ptr_ != nullptr;
  }

  std::exception_ptr Error() {
    assert(HasError());
    return std::exchange(e_ptr_, nullptr);
  }

  T Value() {
    assert(HasValue());
    return std::move(value_.value());
  }

  T Unwrap() {
    if (!value_ && !e_ptr_) {
      support::Terminate("result must have either result or exception");
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

}  // namespace harmony::support
