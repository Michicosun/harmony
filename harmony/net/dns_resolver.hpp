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
  coro::Task<ConnectionParams> Resolve(const std::string& host);

 private:
  coro::Task<std::optional<ConnectionParams>> TryFetchFromCache(
      const std::string& host);

  coro::Task<> SaveToCache(const std::string& host,
                           const ConnectionParams& params);

 private:
  coro::Mutex mutex_;
  std::unordered_map<std::string, ConnectionParams> cache_;
};

}  // namespace harmony::net
