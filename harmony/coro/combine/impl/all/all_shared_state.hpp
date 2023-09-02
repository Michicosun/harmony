#pragma once

#include <cstddef>
#include <tuple>

#include <harmony/coro/combine/impl/all/all_shared_state_base.hpp>
#include <harmony/coro/concepts/awaitable.hpp>
#include <harmony/result/result.hpp>
#include <harmony/runtime/scheduler.hpp>

namespace harmony::coro::impl {

template <class T>
class AllTask;

template <class... Results>
class AllSharedState : public AllSharedStateBase {
 public:
  explicit AllSharedState(AllTask<Results>&&... tasks)
      : AllSharedStateBase(sizeof...(Results)),
        wrapped_tasks_(std::forward<AllTask<Results>>(tasks)...) {
  }

  ~AllSharedState() override = default;

 public:
  void StartTasks(runtime::IScheduler* scheduler) {
    auto ref = ScopedRef();

    SetupParameters(std::index_sequence_for<Results...>{}, scheduler);
    StartTasks(std::index_sequence_for<Results...>{});

    if (consensus_.DoneStartingTasks()) {
      waiter_.resume();
    }
  }

  std::tuple<Results...> Unwrap() {
    UnwrapError();
    return UnwrapResults(std::index_sequence_for<Results...>{});
  }

 private:
  template <size_t... Ids>
  void SetupParameters(std::integer_sequence<size_t, Ids...>,
                       runtime::IScheduler* scheduler) {
    (std::get<Ids>(wrapped_tasks_)
         .SetParameters(scheduler, stop_source_.get_token()),
     ...);
  }

  template <size_t... Ids>
  void StartTasks(std::integer_sequence<size_t, Ids...>) {
    (std::get<Ids>(wrapped_tasks_).Start(this), ...);
  }

  void UnwrapError() {
    if (e_ptr_ != nullptr) {
      std::rethrow_exception(std::exchange(e_ptr_, nullptr));
    }
  }

  template <size_t... Ids>
  std::tuple<Results...> UnwrapResults(std::integer_sequence<size_t, Ids...>) {
    return std::make_tuple(std::get<Ids>(wrapped_tasks_).UnwrapResult()...);
  }

 private:
  std::tuple<AllTask<Results>...> wrapped_tasks_;
};

}  // namespace harmony::coro::impl
