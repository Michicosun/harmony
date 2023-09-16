#pragma once

#include <span>

#include <harmony/coro/core/task.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

class BaseSocket {
 public:
  BaseSocket() = default;
  explicit BaseSocket(io::Fd con_fd);

  ~BaseSocket();

  coro::Task<> Connect(ConnectionParams params, size_t port);

 protected:
  std::optional<io::Fd> con_fd_;
};

class TcpSocket : public BaseSocket {
 public:
  using BaseSocket::BaseSocket;

  coro::Task<size_t> AsyncReadSome(std::span<std::byte> buffer);
  coro::Task<size_t> AsyncWriteSome(std::span<std::byte> buffer);
};

}  // namespace harmony::net
