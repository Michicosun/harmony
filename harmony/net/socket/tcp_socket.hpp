#pragma once

#include <harmony/net/buffer.hpp>
#include <harmony/net/socket/base_socket.hpp>

namespace harmony::net {

class TcpSocket : public BaseSocket {
 public:
  using BaseSocket::BaseSocket;

  coro::Task<> Connect(ConnectionParams params, size_t port);

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
