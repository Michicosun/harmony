#include <netinet/in.h>
#include <sys/socket.h>

#include <harmony/net/net_params.hpp>

namespace harmony::net {

int AddressFamilyToNative(AddressFamily af) {
  switch (af) {
    case AddressFamily::IPv4:
      return AF_INET;
    case AddressFamily::IPv6:
      return AF_INET6;
    case AddressFamily::Unknown:
      return AF_UNSPEC;
  }
}

AddressFamily AddressFamilyFromNative(int native) {
  if (native == AF_INET) {
    return AddressFamily::IPv4;
  } else if (native == AF_INET6) {
    return AddressFamily::IPv6;
  } else {
    return AddressFamily::Unknown;
  }
}

}  // namespace harmony::net
