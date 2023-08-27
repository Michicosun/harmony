#pragma once

#include <atomic>

#include <harmony/support/intrusive/forward_list.hpp>
#include <harmony/support/intrusive/node_unwrap.hpp>

namespace harmony::support {

template <class T>
class ClosableLockFreeQueue {
  using Node = ForwardListNode<T>;

 public:
  bool Push(Node* node) {
    node->next_ = head_.load();

    do {
      if (node->Next() == ClosedHead()) {
        return false;  // closed
      }
    } while (!head_.compare_exchange_weak(node->next_, node));

    return true;
  }

  bool IsClosed() const noexcept {
    return head_.load() == ClosedHead();
  }

  T* Close() {
    return Unwrap(head_.exchange(ClosedHead()));
  }

 private:
  Node* ClosedHead() {
    return reinterpret_cast<Node*>(this);
  }

 private:
  std::atomic<Node*> head_{nullptr};
};

}  // namespace harmony::support
