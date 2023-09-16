#pragma once

#include <harmony/coro/core/task.hpp>
#include <harmony/net/buffer.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/runtime/io/core/epoll_events.hpp>
#include <harmony/runtime/io/core/fd.hpp>
#include <utility>

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

  template <class... Args>
  coro::Task<size_t> AsyncReadSome(Args&&... args) {
    return AsyncReadSome(net::Buffer(std::forward<Args>(args)...));
  }

  template <class... Args>
  coro::Task<size_t> AsyncWriteSome(Args&&... args) {
    return AsyncWriteSome(net::Buffer(std::forward<Args>(args)...));
  }

 private:
  coro::Task<size_t> AsyncReadSome(Buffer buffer);
  coro::Task<size_t> AsyncWriteSome(Buffer buffer);
};

}  // namespace harmony::net
