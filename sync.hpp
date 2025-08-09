#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace synctools {

/**
 * @brief A thread-safe, generic channel implementation.
 *
 * The Channel allows multiple threads to safely exchange data.
 * It uses a mutex and condition variables to handle synchronization
 * for a bounded buffer.
 * @tparam T The type of data to be stored in the channel.
 */
template <typename T>
class Channel {
 private:
  // Mutex to protect shared resources (the buffer).
  std::mutex read_write_lock;
  // Condition variable to signal that the buffer is not empty and can be read from.
  std::condition_variable can_read;
  // Condition variable to signal that the buffer is not full and can be written to.
  std::condition_variable can_write;
  // The underlying buffer to store the elements.
  std::queue<T> buffer;
  // The maximum number of elements the buffer can hold.
  size_t max_size;

  // Checks if the buffer is empty.
  bool is_empty() const { return this->buffer.empty(); }

  // Checks if the buffer is full.
  bool is_full() const { return this->buffer.size() == this->max_size; }

 public:
  /**
   * @brief Constructs a new Channel object with a specified capacity.
   * @param size The maximum number of elements the channel can hold.
   */
  Channel(size_t size) { this->max_size = size; }

  /**
   * @brief Writes a value to the channel.
   *
   * If the channel's buffer is full, the calling thread will block until
   * space becomes available.
   * @param value The value to be written to the channel.
   */
  void write(T value) {
    auto lock = std::unique_lock(this->read_write_lock);

    while (this->is_full()) {
      this->can_write.wait(lock);
    }

    this->buffer.push(value);

    this->can_read.notify_one();
  }

  /**
   * @brief Reads a value from the channel.
   *
   * If the channel's buffer is empty, the calling thread will block until
   * a value becomes available.
   * @return The value read from the channel.
   */
  T read() {
    auto lock = std::unique_lock(this->read_write_lock);

    while (this->is_empty()) {
      this->can_read.wait(lock);
    }

    T popped = this->buffer.front();
    this->buffer.pop();

    this->can_write.notify_one();

    return popped;
  }
};
}  // namespace synctools