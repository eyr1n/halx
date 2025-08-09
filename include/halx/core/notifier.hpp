#pragma once

#include <atomic>
#include <cstdint>

#include "common.hpp"
#include "timeout.hpp"

namespace halx::core {

class Notifier {
public:
  void reset() {
#if __has_include(<cmsis_os2.h>)
    thread_id_ = osThreadGetId();
    osThreadFlagsClear(0xFFFFFFFF);
#else
    flags_.store(0, std::memory_order_relaxed);
#endif
  }

  void set(uint32_t flags) {
#if __has_include(<cmsis_os2.h>)
    osThreadFlagsSet(thread_id_, flags);
#else
    flags_.fetch_or(flags, std::memory_order_relaxed);
#endif
  }

  uint32_t wait(uint32_t flags, uint32_t timeout) {
#if __has_include(<cmsis_os2.h>)
    uint32_t raised =
        osThreadFlagsWait(flags, osFlagsWaitAny | osFlagsNoClear, timeout);
    if ((raised & 0x80000000) != 0) {
      return 0;
    }
    return raised;
#else
    TimeoutHelper timeout_helper{timeout};
    uint32_t raised;
    while ((raised = flags_.load(std::memory_order_relaxed) & flags) == 0) {
      if (timeout_helper.is_timeout()) {
        return 0;
      }
      yield();
    }
    return raised;
#endif
  }

private:
#if __has_include(<cmsis_os2.h>)
  osThreadId_t thread_id_ = nullptr;
#else
  std::atomic<uint32_t> flags_{0};
#endif
};

}; // namespace halx::core