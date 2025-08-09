#pragma once

#include <cstdint>

#include <stm32cubemx_helper/device.hpp>

#if __has_include(<cmsis_os2.h>)
#include <cmsis_os2.h>
#endif

namespace halx::core {

#if __has_include(<cmsis_os2.h>)
inline constexpr uint32_t MAX_DELAY = osWaitForever;
#else
inline constexpr uint32_t MAX_DELAY = HAL_MAX_DELAY;
#endif

inline uint32_t get_tick() {
#if __has_include(<cmsis_os2.h>)
  return osKernelGetTickCount();
#else
  return HAL_GetTick();
#endif
}

inline void delay(uint32_t tick) {
#if __has_include(<cmsis_os2.h>)
  osDelay(tick);
#else
  HAL_Delay(tick);
#endif
}

inline void delay_until(uint32_t tick) {
#if __has_include(<cmsis_os2.h>)
  osDelayUntil(tick);
#else
  int32_t diff = static_cast<int32_t>(tick - HAL_GetTick());
  if (diff > 0) {
    HAL_Delay(diff);
  }
#endif
}

inline void yield() {
#if __has_include(<cmsis_os2.h>)
  osThreadYield();
#else
  __NOP();
#endif
}

} // namespace halx::core
