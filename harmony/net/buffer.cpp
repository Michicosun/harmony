#include <harmony/net/buffer.hpp>

namespace harmony::net {

Buffer::Buffer(std::span<char> buffer) {
  data_ = buffer.data();
  data_size_ = buffer.size();
}

Buffer::Buffer(std::span<char> buffer, size_t size) {
  data_ = buffer.data();
  data_size_ = size;
}

Buffer::Buffer(std::span<std::byte> buffer) {
  data_ = buffer.data();
  data_size_ = buffer.size();
}

Buffer::Buffer(std::span<std::byte> buffer, size_t size) {
  data_ = buffer.data();
  data_size_ = size;
}

std::byte* Buffer::Data() {
  return static_cast<std::byte*>(data_);
}

size_t Buffer::Size() const {
  return data_size_;
}

}  // namespace harmony::net
