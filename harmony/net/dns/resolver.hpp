#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include <harmony/coro/core/task.hpp>
#include <harmony/coro/sync/mutex.hpp>
#include <harmony/net/net_params.hpp>

namespace harmony::net {

class DnsResolver {
 public:
  coro::Task<AddressInfo> Resolve(const std::string& host,
                                  AddressFamily family = AddressFamily::IPv4);

 private:
  coro::Task<std::optional<AddressInfo>> TryFetchFromCache(
      const std::string& host);

  coro::Task<> SaveToCache(const std::string& host, const AddressInfo& params);

 private:
  coro::Mutex mutex_;
  std::unordered_map<std::string, AddressInfo> cache_;
};

}  // namespace harmony::net
