#include <array>
#include <cstddef>
#include <iostream>

#include <harmony/fwd.hpp>

using namespace harmony;  // NOLINT

coro::Task<> worker(net::AcceptInfo ai) {
  std::cout << "connected " << ai.ip_address << ":" << ai.port << std::endl;

  net::TcpSocket socket(ai.fd);
  std::array<std::byte, 1024> buffer;

  while (true) {
    size_t n = co_await socket.AsyncReadSome(buffer);
    co_await socket.AsyncWriteSome({buffer.begin(), buffer.begin() + n});
  }

  co_return {};
}

int main() {
  runtime::Scheduler<executors::ComputeExecutor> scheduler(12);
  scheduler.WithIO();
  scheduler.Start();

  auto amain = [&]() -> coro::Task<> {
    co_await coro::Schedule(scheduler);

    net::Acceptor acceptor(2020);
    coro::TaskGroup tg;

    while (true) {
      auto con = co_await acceptor.Accept();
      co_await tg.Start(worker(std::move(con)));
    }

    co_await tg.Wait();
    co_return {};
  };

  coro::Run(amain());

  scheduler.Stop();
  return 0;
}
