#include <gtest/gtest.h>

#include <harmony/support/intrusive/forward_list.hpp>

class TestNode : public harmony::support::ForwardListNode {
 public:
  explicit TestNode(size_t x)
      : x_{x} {
  }

  size_t Unwrap() const noexcept {
    size_t x = x_;
    delete this;
    return x;
  }

 private:
  size_t x_;
};

TEST(ForwardListTest, Simple) {
  harmony::support::ForwardList list;

  list.PushBack(new TestNode(1));
  list.PushBack(new TestNode(2));
  list.PushBack(new TestNode(3));

  // 1 -> 2 -> 3

  ASSERT_EQ(list.Size(), 3);
  ASSERT_EQ(list.PopFront()->As<TestNode>()->Unwrap(), 1);
  ASSERT_EQ(list.Size(), 2);

  list.PushFront(new TestNode(42));
  list.PushFront(new TestNode(63));

  // 63 -> 42 -> 2 -> 3

  ASSERT_EQ(list.Size(), 4);

  for (size_t i : {63, 42, 2, 3}) {
    ASSERT_EQ(list.PopFront()->As<TestNode>()->Unwrap(), i);
  }

  ASSERT_EQ(list.Size(), 0);
  ASSERT_EQ(list.IsEmpty(), true);
}
