#include <netinet/in.h>

#include <harmony/net/net_params.hpp>

namespace harmony::net {

int AddressFamilyToNative(AddressFamily af) {
  switch (af) {
    case AddressFamily::IPv4:
      return AF_INET;
    case AddressFamily::IPv6:
      return AF_INET6;
  }
}

}  // namespace harmony::net
