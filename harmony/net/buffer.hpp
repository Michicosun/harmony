#pragma once

#include <span>

namespace harmony::net {

class Buffer {
 public:
  explicit Buffer(std::span<char> buffer);
  explicit Buffer(std::span<char> buffer, size_t size);

  explicit Buffer(std::span<std::byte> buffer);
  explicit Buffer(std::span<std::byte> buffer, size_t size);

  std::byte* Data();
  size_t Size() const;

 private:
  void* data_;        // data stored in bytes
  size_t data_size_;  // data bytes size
};

}  // namespace harmony::net
