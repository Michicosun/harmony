#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <harmony/coro/events/fd_ready.hpp>
#include <harmony/net/acceptor.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/runtime/io/core/fd.hpp>

namespace harmony::net {

Acceptor::Acceptor(size_t port, AddressFamily af) {
  PrepareListeningFd(port, af);
}

Acceptor::~Acceptor() {
  close(lfd_);
}

coro::Task<AcceptInfo> Acceptor::Accept() {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  auto status = co_await coro::FdReady(lfd_, io::Operation::Read);

  if (status == io::EventStatus::Error) {
    throw NetError("error while polling descriptor");
  }

  if (status == io::EventStatus::Closed) {
    throw NetError("descriptor is closed");
  }

  int client_fd =
      accept(lfd_, (struct sockaddr*)&client_addr, &client_addr_len);

  if (client_fd < 0) {
    throw NetError(strerror(errno));
  }

  io::MakeNonblocking(client_fd);

  co_return AcceptInfo{
      .fd = client_fd,
      .ip_address = inet_ntoa(client_addr.sin_addr),
      .port = client_addr.sin_port,
  };
}

void Acceptor::PrepareListeningFd(size_t port, AddressFamily af) {
  lfd_ = socket(AddressFamilyToNative(af), SOCK_STREAM, 0);
  if (lfd_ < 0) {
    throw NetError(strerror(errno));
  }

  const int enable = 1;
  if (setsockopt(lfd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable,
                 sizeof(int)) < 0) {
    throw NetError(strerror(errno));
  }

  struct sockaddr_in listen_addr = {};
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AddressFamilyToNative(af);
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  listen_addr.sin_port = htons(port);

  if (bind(lfd_, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
    throw NetError(strerror(errno));
  }

  if (listen(lfd_, 5) < 0) {
    throw NetError(strerror(errno));
  }

  io::MakeNonblocking(lfd_);
}

}  // namespace harmony::net
