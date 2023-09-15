#pragma once

#include <span>

#include <harmony/coro/core/task.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

class TcpSocket {
 public:
  TcpSocket() = default;
  explicit TcpSocket(io::Fd con_fd);

  ~TcpSocket();

  coro::Task<> Connect(ConnectionParams params, size_t port);

  coro::Task<size_t> AsyncReadSome(std::span<std::byte> buffer);
  coro::Task<size_t> AsyncWriteSome(std::span<std::byte> buffer);

 private:
  std::optional<io::Fd> con_fd_;
};

}  // namespace harmony::net
