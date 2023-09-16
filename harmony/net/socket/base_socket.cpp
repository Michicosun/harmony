#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/socket/base_socket.hpp>

namespace harmony::net {

BaseSocket::BaseSocket(io::Fd con_fd)
    : con_fd_{con_fd} {
}

BaseSocket::~BaseSocket() {
  if (con_fd_.has_value()) {
    close(con_fd_.value());
  }
}

int BaseSocket::SocketTypeToNative(SocketType type) {
  switch (type) {
    case SocketType::Tcp:
      return SOCK_STREAM;
    case SocketType::Udp:
      return SOCK_DGRAM;
  }
}

coro::Task<> BaseSocket::Connect(SocketType type, ConnectionParams params,
                                 size_t port) {
  if (con_fd_.has_value()) {
    throw NetError("socket is already connected to endpoint");
  }

  struct sockaddr_storage ss = {};
  socklen_t addrlen;

  int family = AddressFamilyToNative(params.address_family);
  switch (family) {
    case AF_INET: {
      struct sockaddr_in* addr = (struct sockaddr_in*)&ss;
      addr->sin_family = AF_INET;
      addr->sin_port = htons(port);
      inet_pton(AF_INET, params.ip_address.c_str(), &addr->sin_addr);
      addrlen = sizeof(struct sockaddr_in);
      break;
    }

    case AF_INET6: {
      struct sockaddr_in6* addr = (struct sockaddr_in6*)&ss;
      addr->sin6_family = AF_INET6;
      addr->sin6_port = htons(port);
      inet_pton(AF_INET6, params.ip_address.c_str(), &addr->sin6_addr);
      addrlen = sizeof(struct sockaddr_in6);
      break;
    }
  }

  // create socket
  con_fd_ = socket(family, SocketTypeToNative(type), 0);
  if (con_fd_ < 0) {
    throw NetError(strerror(errno));
  }

  // convert socket fd to nonblocking mode
  io::MakeNonblocking(con_fd_.value());

  // make connection
  int code = connect(con_fd_.value(), (struct sockaddr*)&ss, addrlen);

  if (code < 0 &&
      (errno == EAGAIN || errno == EALREADY || errno == EINPROGRESS)) {
    co_await coro::FdReady(con_fd_.value(), io::Operation::Write);

    int sockopt_error;
    socklen_t optlen = sizeof(sockopt_error);
    if (getsockopt(con_fd_.value(), SOL_SOCKET, SO_ERROR, &sockopt_error,
                   &optlen) < 0) {
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

}  // namespace harmony::net