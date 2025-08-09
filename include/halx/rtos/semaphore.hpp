#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>

#include <cmsis_os2.h>

namespace halx::rtos {

class Semaphore {
private:
  struct Deleter {
    void operator()(osSemaphoreId_t queue_id) { osSemaphoreDelete(queue_id); }
  };

  using SemaphoreId =
      std::unique_ptr<std::remove_pointer_t<osSemaphoreId_t>, Deleter>;

public:
  Semaphore(uint32_t max, uint32_t initial, uint32_t attr_bits = 0) {
    osSemaphoreAttr_t attr{};
    attr.attr_bits = attr_bits;
    semaphore_id_ = SemaphoreId{osSemaphoreNew(max, initial, &attr)};
  }

  bool acquire(uint32_t timeout) {
    return osSemaphoreAcquire(semaphore_id_.get(), timeout) == osOK;
  }

  void acquire() { acquire(osWaitForever); }

  void release() { osSemaphoreRelease(semaphore_id_.get()); }

private:
  SemaphoreId semaphore_id_;
};

} // namespace halx::rtos
