#pragma once

namespace harmony::net {

enum class AddressFamily {
  IPv4,
  IPv6,
};

int AddressFamilyToNative(AddressFamily af);

}  // namespace harmony::net
