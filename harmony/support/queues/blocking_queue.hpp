#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>

#include <harmony/support/intrusive/forward_list.hpp>

namespace harmony::support {

template <class T>
class UnboundedBlockingQueue {
  using Node = ForwardListNode<T>;

 public:
  bool Push(Node* node) {
    std::lock_guard guard(mutex_);
    if (closed_) {
      return false;
    }

    buffer_.PushBack(node);
    empty_queue_.notify_one();

    return true;
  }

  std::optional<T*> TakeOne() {
    std::unique_lock guard(mutex_);
    while (!closed_ && buffer_.IsEmpty()) {
      empty_queue_.wait(guard);
    }

    if (!buffer_.IsEmpty()) {
      return buffer_.PopFront();
    }

    return std::nullopt;
  }

  void Close() {
    std::lock_guard guard(mutex_);
    closed_ = true;
    empty_queue_.notify_all();
  }

  bool IsClosed() {
    std::lock_guard guard(mutex_);
    return closed_;
  }

 private:
  bool closed_{false};
  ForwardList<T> buffer_;
  std::mutex mutex_;
  std::condition_variable empty_queue_;
};

}  // namespace harmony::support
