#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/net_params.hpp>
#include <harmony/net/socket/tcp_socket.hpp>

namespace harmony::net {

TcpSocket::TcpSocket(AddressFamily family)
    : family_{family} {
  con_fd_ = socket(AddressFamilyToNative(family), SOCK_STREAM, 0);
  if (con_fd_ < 0) {
    throw NetError(strerror(errno));
  }

  io::MakeNonblocking(con_fd_);
}

TcpSocket::TcpSocket(io::Fd con_fd)
    : con_fd_{con_fd} {
}

TcpSocket::~TcpSocket() {
  close(con_fd_);
}

coro::Task<> TcpSocket::Connect(std::string host, size_t port) {
  if (!family_.has_value()) {
    throw NetError("socket was created using fd, expected to be connected");
  }

  struct sockaddr_storage ss = {};
  socklen_t addrlen;

  int family = AddressFamilyToNative(family_.value());
  switch (family) {
    case AF_INET: {
      struct sockaddr_in* addr = (struct sockaddr_in*)&ss;
      addr->sin_family = AF_INET;
      addr->sin_port = htons(port);
      inet_pton(AF_INET, host.c_str(), &addr->sin_addr);
      addrlen = sizeof(struct sockaddr_in);
      break;
    }

    case AF_INET6: {
      struct sockaddr_in6* addr = (struct sockaddr_in6*)&ss;
      addr->sin6_family = AF_INET6;
      addr->sin6_port = htons(port);
      inet_pton(AF_INET6, host.c_str(), &addr->sin6_addr);
      addrlen = sizeof(struct sockaddr_in6);
      break;
    }
  }

  // make connection
  int code = connect(con_fd_, (struct sockaddr*)&ss, addrlen);

  if (code < 0 &&
      (errno == EAGAIN || errno == EALREADY || errno == EINPROGRESS)) {
    co_await coro::FdReady(con_fd_, io::Operation::Write);

    int sockopt_error;
    socklen_t optlen = sizeof(sockopt_error);

    code = getsockopt(con_fd_, SOL_SOCKET, SO_ERROR, &sockopt_error, &optlen);

    if (code < 0) {
      throw NetError(strerror(errno));
    }

    if (sockopt_error != 0) {
      throw NetError(strerror(sockopt_error));
    }

  } else if (code < 0) {
    throw NetError(strerror(errno));
  }

  co_return {};
}

coro::Task<size_t> TcpSocket::ReadSome(Buffer buffer) {
  auto status = co_await coro::FdReady(con_fd_, io::Operation::Read);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    throw NetError("socket was closed from the other side");
  }

  int64_t n = read(con_fd_, buffer.Data(), buffer.Size());

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return n;
}

coro::Task<size_t> TcpSocket::WriteSome(Buffer buffer) {
  auto status = co_await coro::FdReady(con_fd_, io::Operation::Write);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    throw NetError("socket was closed from the other side");
  }

  int64_t n = write(con_fd_, buffer.Data(), buffer.Size());

  if (n < 0) {
    throw NetError(strerror(errno));
  }

  co_return n;
}

}  // namespace harmony::net
