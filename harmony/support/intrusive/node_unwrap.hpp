#pragma once

#include <harmony/support/intrusive/forward_list.hpp>

namespace harmony::support {

template <class T>
T* Unwrap(ForwardListNode<T>* task_ptr) {
  if (task_ptr != nullptr) {
    return task_ptr->Unwrap();
  }

  return nullptr;
}

}  // namespace harmony::support
