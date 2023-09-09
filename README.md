Minimalistic concurrency library built on top of С++20 coroutines

---

# Building

This project requires building only for linux platform.

## Building from source

Clone repository with all submodules, there is only one: gtest, that is necessary for tests

```bash
git clone --recurse-submodules https://github.com/Michicosun/harmony.git
```

Project uses cmake for building

```bash
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 12
```

### CMake Options

| Option | Description |
|---------- |------|
| BUILD_TESTS | builds tests |
| BUILD_EXAMPLES | builds examples |
| BUILD_PLAYGROUND | builds playground |

## Linking via CMake FetchContent

Project library target is harmony

```cmake
FetchContent_Declare(
        harmony
        GIT_REPOSITORY https://github.com/Michicosun/harmony.git
        GIT_TAG TAG
)
FetchContent_MakeAvailable(harmony)
target_link_libraries(TARGET harmony)
```

# Synchronization primitives

## Mutex

Base asynchronous mutex for coroutine runtime, usage:

```cpp
for (size_t j = 0; j < 100; ++j) {
    auto lock = co_await mutex.ScopedLock();
    counter += 1;
}
```

returns a class similar to std::unique_lock with unlock in destructor

## OneShotEvent

This is Single Consumer Multi Producer event for coroutine runtime, usage:

```cpp
auto coroutine = [&]() -> coro::Task<> {
    co_await coro::Schedule(scheduler);
    event.Complete(); // completes event and wake ups waiters
    co_return {};
};

coro::Detach(coroutine());
co_await event.Wait(); // waits for event asynchronously
```

## WaitGroup

Class with similar semantics as Golang WaitGroup, usage:

```cpp
coro::WaitGroup wg;
wg.Add(20); // makes balance equal to 20

auto runner = [&]() -> coro::Task<> {
    co_await coro::Yield();
    wg.Done(); // decreases balance by 1
    co_return {};
};

for (size_t i = 0; i < coro_count; ++i) {
    coro::Detach(runner());
}

co_await wg.Wait(); // waits until balance is dropped to 0
```

# Сombinators

Сombinators are main blocks for cancellation, we are using structures concurrency to prevent leaks and nondeterministic execution, to achieve that every combinator must contain stop_source and all sources will be linked in dynamic runtime tree of awaiters.

If some coroutine trigger stop, it will eventually stop all subtrees of this coroutine runtime tree node.

## All

All - combinator that runs in parallel a bunch of coroutines and awaits all of them or throws first occurred error, example of usage:

```cpp
auto task1 = []() -> coro::Task<size_t> {
    co_await coro::Yield();  // change thread
    co_await coro::SleepFor(500ms);
    co_return 10;
};

auto task2 = []() -> coro::Task<std::string> {
    co_await coro::Yield();  // change thread
    co_await coro::SleepFor(1000ms);
    co_return "hello";
};

auto result = co_await coro::All(task1(), task2()); // will return tuple
```

## First

First - combinator that returns first completed coroutine result of throws the last occurred error, usage is similar to All combinator.

It is the best way to do timeouts for queries:

```cpp
auto long_running_query = []() -> coro::Task<size_t> {
    co_await coro::SleepFor(10s);
    co_return 1;
};

auto timeout = []() -> coro::Task<size_t> {
    co_await coro::SleepFor(500ms);
    co_return 2;
};

auto result = co_await coro::First(long_running_query(), timeout());
```

In this example, a long-running query will be cancelled after first returns.

## TaskGroup

TaskGroup contains a bunch of running coroutines and can be dynamically added a new one, but you can wait for the completion of all coroutines only 1 time, example:

```cpp
coro::TaskGroup tg;

while (true) {
    co_await tg.Start(coroutine());
}

co_await tg.Wait();
```

# Runtime

Harmony runtime consists of one scheduler that contains timer event source, io event source and main executor. You can configure many schedulers in the program with different settings and event sources as well, example:

```cpp
runtime::Scheduler<executors::ComputeExecutor> scheduler(/*threads=*/12);
scheduler.WithIO();
scheduler.WithTimer();
scheduler.Start();
```
