#pragma once

#include <harmony/coro/combine/impl/first/first_shared_state_base.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/runtime/scheduler.hpp>
#include <harmony/support/result/result.hpp>

namespace harmony::coro::impl {

template <class T>
class FirstTask;

template <class T, class... Tasks>
class FirstSharedState : public FirstSharedStateBase<T> {
 public:
  explicit FirstSharedState(Tasks&&... tasks)
      : FirstSharedStateBase<T>(sizeof...(Tasks)),
        wrapped_tasks_(std::forward<FirstTask<T>>(tasks)...) {
  }

  ~FirstSharedState() override = default;

 public:
  void StartTasks(runtime::IScheduler* scheduler) {
    auto ref = FirstSharedStateBase<T>::ScopedRef();

    SetupParameters(std::index_sequence_for<Tasks...>{}, scheduler);
    StartTasks(std::index_sequence_for<Tasks...>{});

    if (FirstSharedStateBase<T>::consensus_.DoneStartingTasks()) {
      FirstSharedStateBase<T>::stop_callback_.reset();
      FirstSharedStateBase<T>::waiter_.resume();
    }
  }

  T Unwrap() {
    return FirstSharedStateBase<T>::result_.Unwrap();
  }

 private:
  template <size_t... Ids>
  void SetupParameters(std::integer_sequence<size_t, Ids...>,
                       runtime::IScheduler* scheduler) {
    (std::get<Ids>(wrapped_tasks_)
         .SetParameters(scheduler,
                        FirstSharedStateBase<T>::stop_source_.get_token()),
     ...);
  }

  template <size_t... Ids>
  void StartTasks(std::integer_sequence<size_t, Ids...>) {
    (std::get<Ids>(wrapped_tasks_).Start(this), ...);
  }

 private:
  std::tuple<Tasks...> wrapped_tasks_;
};

}  // namespace harmony::coro::impl
