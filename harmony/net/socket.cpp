#include <sys/socket.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/socket.hpp>

namespace harmony::net {

TcpSocket::TcpSocket(io::Fd con_fd)
    : con_fd_{con_fd} {
}

TcpSocket::~TcpSocket() {
  close(con_fd_);
}

coro::Task<TcpSocket::Result> TcpSocket::AsyncReadSome(
    std::span<std::byte> buffer) {
  auto status = co_await coro::FdReady(con_fd_, io::Operation::Read);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    co_return Result{
        .status = status,
        .bytes_transferred = 0,
    };
  }

  int64_t n = read(con_fd_, buffer.data(), buffer.size());

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return Result{
      .status = status,
      .bytes_transferred = static_cast<size_t>(n),
  };
}

coro::Task<TcpSocket::Result> TcpSocket::AsyncWriteSome(
    std::span<std::byte> buffer) {
  auto status = co_await coro::FdReady(con_fd_, io::Operation::Write);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    co_return Result{
        .status = status,
        .bytes_transferred = 0,
    };
  }

  int64_t n = write(con_fd_, buffer.data(), buffer.size());

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return Result{
      .status = status,
      .bytes_transferred = static_cast<size_t>(n),
  };
}

}  // namespace harmony::net
