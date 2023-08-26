#pragma once

#include <utility>

namespace harmony::support {

class ForwardListNode {
 public:
  ForwardListNode* Next() const noexcept;
  void SetNext(ForwardListNode* node) noexcept;

  void Reset() noexcept;

 private:
  ForwardListNode* next_{nullptr};
};

class ForwardList {
 public:
  void PushBack(ForwardListNode* node) noexcept;
  void PushFront(ForwardListNode* node) noexcept;

  ForwardListNode* PopFront() noexcept;

  bool IsEmpty() const noexcept;
  size_t Size() const noexcept;

 private:
  size_t size_{0};
  ForwardListNode* head_{nullptr};
  ForwardListNode* tail_{nullptr};
};

};  // namespace harmony::support
