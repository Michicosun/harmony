#pragma once

#include <atomic>

#include <harmony/support/intrusive/forward_list.hpp>
#include <harmony/support/intrusive/node_unwrap.hpp>

namespace harmony::support {

template <class T>
class BatchLockFreeQueue {
  using Node = ForwardListNode<T>;

 public:
  void Push(Node* node) {
    node->next = head_.load();
    while (!head_.compare_exchange_weak(node->next, node)) {
    }
  }

  T* ExtractAll() {
    return Unwrap(head_.exchange(nullptr));
  }

 private:
  std::atomic<Node*> head_{nullptr};
};

}  // namespace harmony::support
