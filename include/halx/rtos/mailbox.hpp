#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include <FreeRTOS.h>
#include <queue.h>

namespace halx::rtos {

template <class T> class Mailbox {
private:
  struct Deleter {
    void operator()(QueueHandle_t queue_handle) { vQueueDelete(queue_handle); }
  };

  using QueueHandle =
      std::unique_ptr<std::remove_pointer_t<QueueHandle_t>, Deleter>;

public:
  Mailbox() { queue_handle_ = QueueHandle{xQueueCreate(1, sizeof(T))}; }

  bool push(const T &value, uint32_t timeout = 0) {
    if (timeout != 0) {
      return false;
    }
    if (xPortIsInsideInterrupt() == pdTRUE) {
      BaseType_t yield = pdFALSE;
      if (xQueueOverwriteFromISR(queue_handle_.get(), &value, &yield) !=
          pdTRUE) {
        return false;
      }
      portYIELD_FROM_ISR(yield);
      return true;
    }
    return xQueueOverwrite(queue_handle_.get(), &value) == pdTRUE;
  }

  std::optional<T> pop(uint32_t timeout = 0) {
    T value;
    if (xPortIsInsideInterrupt() == pdTRUE) {
      if (timeout != 0) {
        return std::nullopt;
      }
      BaseType_t yield = pdFALSE;
      if (xQueueReceiveFromISR(queue_handle_.get(), &value, &yield) != pdTRUE) {
        return std::nullopt;
      }
      portYIELD_FROM_ISR(yield);
      return value;
    }
    if (xQueueReceive(queue_handle_.get(), &value, timeout) != pdTRUE) {
      return std::nullopt;
    }
    return value;
  }

  std::optional<T> peek(uint32_t timeout = 0) const {
    T value;
    if (xPortIsInsideInterrupt() == pdTRUE) {
      if (timeout != 0) {
        return std::nullopt;
      }
      if (xQueuePeekFromISR(queue_handle_.get(), &value) != pdTRUE) {
        return std::nullopt;
      }
      return value;
    }
    if (xQueuePeek(queue_handle_.get(), &value, timeout) != pdTRUE) {
      return std::nullopt;
    }
    return value;
  }

  void clear() { xQueueReset(queue_handle_.get()); }

  size_t size() const {
    if (xPortIsInsideInterrupt() == pdTRUE) {
      return uxQueueMessagesWaitingFromISR(queue_handle_.get());
    }
    return uxQueueMessagesWaiting(queue_handle_.get());
  }

  size_t capacity() const { return 1; }

private:
  QueueHandle queue_handle_;
};

} // namespace halx::rtos
