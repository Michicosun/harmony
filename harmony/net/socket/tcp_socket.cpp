#include <unistd.h>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/net/socket/tcp_socket.hpp>
#include "harmony/net/socket/base_socket.hpp"

namespace harmony::net {

coro::Task<> TcpSocket::Connect(ConnectionParams params, size_t port) {
  return BaseSocket::Connect(SocketType::Tcp, std::move(params), port);
}

coro::Task<size_t> TcpSocket::AsyncReadSome(Buffer buffer) {
  if (!con_fd_.has_value()) {
    throw NetError("socket not connected to endpoint");
  }

  auto status = co_await coro::FdReady(con_fd_.value(), io::Operation::Read);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    throw NetError("socket was closed from the other side");
  }

  int64_t n = read(con_fd_.value(), buffer.Data(), buffer.Size());

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return n;
}

coro::Task<size_t> TcpSocket::AsyncWriteSome(Buffer buffer) {
  if (!con_fd_.has_value()) {
    throw NetError("socket not connected to endpoint");
  }

  auto status = co_await coro::FdReady(con_fd_.value(), io::Operation::Write);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    throw NetError("socket was closed from the other side");
  }

  int64_t n = write(con_fd_.value(), buffer.Data(), buffer.Size());

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return n;
}

}  // namespace harmony::net
