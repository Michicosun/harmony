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
    node->next = head_.load();

    do {
      if (node->Next() == ClosedHead()) {
        return false;  // closed
      }
    } while (!head_.compare_exchange_weak(node->next, node));

    return true;
  }

  bool IsClosed() const noexcept {
    return head_.load() == ClosedHead();
  }

  T* Close() {
    return Unwrap(head_.exchange(ClosedHead()));
  }

 private:
  const Node* ClosedHead() const {
    return reinterpret_cast<const Node*>(this);
  }

  Node* ClosedHead() {
    return reinterpret_cast<Node*>(this);
  }

 private:
  std::atomic<Node*> head_{nullptr};
};

}  // namespace harmony::support
