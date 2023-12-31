#include <netdb.h>
#include <cstring>

#include <harmony/coro/events/sleep_for.hpp>
#include <harmony/net/dns/resolver.hpp>
#include <harmony/net/exceptions.hpp>
#include <harmony/net/net_params.hpp>

using namespace std::chrono_literals;

namespace harmony::net {

coro::Task<AddressInfo> DnsResolver::Resolve(const std::string& host,
                                             AddressFamily family) {
  // first try to get from cache
  if (auto cached_result = co_await TryFetchFromCache(host)) {
    co_return cached_result.value();
  }

  // configure request
  struct gaicb req = {};
  struct addrinfo req_info = {};
  memset(&req, 0, sizeof(req));
  memset(&req_info, 0, sizeof(req_info));

  req_info.ai_family = AddressFamilyToNative(family);
  req.ar_name = host.c_str();
  req.ar_request = &req_info;

  struct gaicb* req_ptr[] = {&req};

  // create async request
  int gai_status = getaddrinfo_a(GAI_NOWAIT, req_ptr, 1, nullptr);
  if (gai_status < 0) {
    throw NetError(gai_strerror(gai_status));
  }

  // poll until request ready
  while (true) {
    int op_status = gai_error(&req);
    if (op_status == 0) {
      break;
    } else if (op_status == EAI_INPROGRESS) {
      co_await coro::SleepFor(1ms);
    } else {
      throw NetError(gai_strerror(op_status));
    }
  }

  // parse ip-address
  struct addrinfo* result = req.ar_result;
  char ip_address[NI_MAXHOST];

  int gni_status = getnameinfo(result->ai_addr, result->ai_addrlen, ip_address,
                               sizeof(ip_address), nullptr, 0, NI_NUMERICHOST);
  if (gni_status != 0) {
    throw NetError(gai_strerror(gni_status));
  }

  AddressInfo params{
      .address_family = AddressFamilyFromNative(result->ai_family),
      .ip_address = ip_address,
  };

  // save to cache
  co_await SaveToCache(host, params);
  co_return params;
}

coro::Task<std::optional<AddressInfo>> DnsResolver::TryFetchFromCache(
    const std::string& host) {
  auto lock = co_await mutex_.ScopedLock();

  if (auto it = cache_.find(host); it != cache_.end()) {
    co_return it->second;
  }

  co_return std::nullopt;
}

coro::Task<> DnsResolver::SaveToCache(const std::string& host,
                                      const AddressInfo& params) {
  auto lock = co_await mutex_.ScopedLock();
  cache_[host] = params;
  co_return {};
}

}  // namespace harmony::net
