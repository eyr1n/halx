#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include <cmsis_os2.h>

namespace halx::rtos {

template <class T> class Queue {
private:
  struct Deleter {
    void operator()(osMessageQueueId_t queue_id) {
      osMessageQueueDelete(queue_id);
    }
  };

  using QueueId =
      std::unique_ptr<std::remove_pointer_t<osMessageQueueId_t>, Deleter>;

public:
  Queue(size_t capacity, uint32_t attr_bits = 0) {
    osMessageQueueAttr_t attr{};
    attr.attr_bits = attr_bits;
    queue_id_ = QueueId{osMessageQueueNew(capacity, sizeof(T), &attr)};
  }

  bool push(const T &value, uint32_t timeout) {
    return osMessageQueuePut(queue_id_.get(), &value, 0, timeout) == osOK;
  }

  bool push(const T &value) { return push(value, 0); }

  std::optional<T> pop(uint32_t timeout) {
    T value;
    if (osMessageQueueGet(queue_id_.get(), &value, nullptr, timeout) != osOK) {
      return std::nullopt;
    }
    return value;
  }

  std::optional<T> pop() { return pop(0); }

  bool pop(T &value, uint32_t timeout) {
    if (auto a = pop(timeout)) {
      value = *a;
      return true;
    }
    return false;
  }

  void clear() { osMessageQueueReset(queue_id_.get()); }

  size_t size() const { return osMessageQueueGetCount(queue_id_.get()); }

  size_t capacity() const { return osMessageQueueGetCapacity(queue_id_.get()); }

private:
  QueueId queue_id_;
};

} // namespace halx::rtos
