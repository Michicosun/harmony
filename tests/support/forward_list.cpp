#include <gtest/gtest.h>

#include <harmony/support/intrusive/forward_list.hpp>

class TestNode : public harmony::support::ForwardListNode<TestNode> {
 public:
  explicit TestNode(size_t x)
      : x_{x} {
  }

  size_t Destroy() const noexcept {
    size_t x = x_;
    delete this;
    return x;
  }

 private:
  size_t x_;
};

TEST(ForwardListTest, Simple) {
  harmony::support::ForwardList<TestNode> list;

  list.PushBack(new TestNode(1));
  list.PushBack(new TestNode(2));
  list.PushBack(new TestNode(3));

  // 1 -> 2 -> 3

  ASSERT_EQ(list.Size(), 3);
  ASSERT_EQ(list.PopFront()->Unwrap()->Destroy(), 1);
  ASSERT_EQ(list.Size(), 2);

  list.PushFront(new TestNode(42));
  list.PushFront(new TestNode(63));

  // 63 -> 42 -> 2 -> 3

  ASSERT_EQ(list.Size(), 4);

  for (size_t i : {63, 42, 2, 3}) {
    ASSERT_EQ(list.PopFront()->Unwrap()->Destroy(), i);
  }

  ASSERT_EQ(list.Size(), 0);
  ASSERT_EQ(list.IsEmpty(), true);
}
