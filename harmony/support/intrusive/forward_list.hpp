#pragma once

#include <cstddef>

namespace harmony::support {

template <class T>
struct ForwardListNode {
  ForwardListNode* Next() const noexcept {
    return next;
  }

  void SetNext(ForwardListNode* node) noexcept {
    next = node;
  }

  void Reset() noexcept {
    next = nullptr;
  }

  T* Unwrap() {
    return static_cast<T*>(this);
  }

  ForwardListNode* next{nullptr};
};

// head -> * -> * -> ... -> * -> tail -> nullptr

template <class T>
class ForwardList {
  using Node = ForwardListNode<T>;

 public:
  void PushBack(Node* node) noexcept {
    node->Reset();

    if (IsEmpty()) {
      head_ = tail_ = node;
    } else {
      tail_->SetNext(node);
      tail_ = node;
    }

    ++size_;
  }

  void PushFront(Node* node) noexcept {
    node->Reset();

    if (IsEmpty()) {
      head_ = tail_ = node;
    } else {
      node->SetNext(head_);
      head_ = node;
    }

    ++size_;
  }

  T* PopFront() noexcept {
    if (IsEmpty()) {
      return nullptr;
    }

    Node* removed = head_;

    if (head_ == tail_) {
      head_ = tail_ = nullptr;
    } else {
      head_ = head_->Next();
    }

    --size_;
    removed->Reset();

    return removed->Unwrap();
  }

  bool IsEmpty() const noexcept {
    return head_ == nullptr;
  }

  size_t Size() const noexcept {
    return size_;
  }

 private:
  size_t size_{0};
  Node* head_{nullptr};
  Node* tail_{nullptr};
};

};  // namespace harmony::support
