#include <gtest/gtest.h>

#include <harmony/runtime/executors/manual/executor.hpp>
#include <harmony/runtime/executors/task.hpp>

struct TestTask : public harmony::executors::TaskBase {
  size_t& var;
  size_t x;

  TestTask(size_t& var, size_t x)
      : var(var),
        x(x) {
  }

  void Run() noexcept override {
    var = x;
    delete this;
  }
};

TEST(ManualExecutorTest, Simple) {
  harmony::executors::ManualExecutor manual;

  size_t step = 0;

  ASSERT_TRUE(manual.IsEmpty());
  ASSERT_FALSE(manual.RunNext());
  ASSERT_EQ(manual.Run(99), 0);

  manual.Submit(new TestTask(step, 1));
  ASSERT_FALSE(manual.IsEmpty());
  ASSERT_EQ(manual.TasksInQueue(), 1);
  ASSERT_EQ(step, 0);

  manual.Submit(new TestTask(step, 2));
  ASSERT_EQ(manual.TasksInQueue(), 2);
  ASSERT_EQ(step, 0);

  ASSERT_TRUE(manual.RunNext());
  ASSERT_EQ(step, 1);

  ASSERT_FALSE(manual.IsEmpty());
  ASSERT_EQ(manual.TasksInQueue(), 1);

  manual.Submit(new TestTask(step, 3));
  ASSERT_EQ(manual.TasksInQueue(), 2);

  ASSERT_EQ(manual.Run(100), 2);
  ASSERT_EQ(step, 3);

  ASSERT_TRUE(manual.IsEmpty());
  ASSERT_FALSE(manual.RunNext());
}
