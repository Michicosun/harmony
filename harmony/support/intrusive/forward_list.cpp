#include <harmony/support/intrusive/forward_list.hpp>
#include <utility>

namespace harmony::support {

ForwardListNode* ForwardListNode::Next() const noexcept {
  return next_;
}

void ForwardListNode::SetNext(ForwardListNode* node) noexcept {
  next_ = node;
}

void ForwardListNode::Reset() noexcept {
  next_ = nullptr;
}

// head -> * -> * -> ... -> * -> tail -> nullptr

void ForwardList::PushBack(ForwardListNode* node) noexcept {
  node->Reset();

  if (IsEmpty()) {
    head_ = tail_ = node;
  } else {
    tail_->SetNext(node);
    tail_ = node;
  }

  ++size_;
}

void ForwardList::PushFront(ForwardListNode* node) noexcept {
  node->Reset();

  if (IsEmpty()) {
    head_ = tail_ = node;
  } else {
    node->SetNext(head_);
    head_ = node;
  }

  ++size_;
}

ForwardListNode* ForwardList::PopFront() noexcept {
  if (IsEmpty()) {
    return nullptr;
  }

  ForwardListNode* removed = head_;

  if (head_ == tail_) {
    head_ = tail_ = nullptr;
  } else {
    head_ = head_->Next();
  }

  --size_;
  removed->Reset();

  return removed;
}

bool ForwardList::IsEmpty() const noexcept {
  return head_ == nullptr;
}

size_t ForwardList::Size() const noexcept {
  return size_;
}

};  // namespace harmony::support
