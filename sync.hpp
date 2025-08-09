#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace synctools {

// Internal implementation detail to unpack a container and call a function with its elements.
// This function is not meant to be called directly.
template <typename Func, typename C, std::size_t... Is>
auto unpack_call_impl(Func&& func, const C& container,
                      std::index_sequence<Is...>) -> decltype(auto) {
  return std::forward<Func>(func)(container[Is]...);
}

// Public interface to unpack a container and call a function with its elements.
// It deduces the container's size and generates the index sequence.
template <typename Func, typename C>
auto unpack_call(Func&& func, const C& container) -> decltype(auto) {
  return unpack_call_impl(std::forward<Func>(func), container,
                          std::make_index_sequence<container.size()>{});
}


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
  // Condition variable to signal that the buffer is not empty and can be read
  // from.
  std::condition_variable can_read;
  // Condition variable to signal that the buffer is not full and can be written
  // to.
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

/**
 * @brief A class that wraps a function call with a scoped lock on multiple mutexes.
 *
 * It uses template metaprogramming to lock a variable number of mutexes
 * provided in a container before calling the wrapped function.
 *
 * @tparam Func The type of the function to be wrapped.
 * @tparam Return The return type of the function.
 * @tparam Args The types of the arguments passed to the function.
 */
template <typename Func, typename Return, typename... Args>
class ParallelFunction {
  // Alias for a shared pointer to a mutex.
  using SPtrMutex = std::shared_ptr<std::mutex>;

 private:
  // A vector of shared pointers to mutexes to be locked.
  std::vector<SPtrMutex> mutexes;
  // The function to be executed after the mutexes are locked.
  Func function;

 public:
  /**
   * @brief Constructs a new ParallelFunction object.
   *
   * @param mtxs A vector of shared pointers to mutexes.
   * @param inner The function to be wrapped.
   */
  ParallelFunction(const std::vector<SPtrMutex>& mtxs, Func inner) {
    this->mutexes = mtxs;
    this->function = inner;
  }

  /**
   * @brief The function call operator.
   *
   * It locks all mutexes in the `mutexes` vector and then
   * calls the wrapped function with the provided arguments. The locks are
   * automatically released when this function returns.
   *
   * @param args The arguments to forward to the wrapped function.
   * @return The return value of the wrapped function.
   */
  Return operator()(Args&&... args) {
    unpack_call([](auto&&... locks) { std::scoped_lock lock(locks...); },
                this->mutexes);
    return this->function(std::forward(args)...);
  }
};

}  // namespace synctools