#pragma once

#include <cstdint>

#include "halx/core.hpp"

namespace halx::peripheral {

#ifdef HAL_TIM_MODULE_ENABLED

class Pwm {
public:
  Pwm(TIM_HandleTypeDef *handle, uint32_t channel)
      : handle_{handle}, channel_{channel} {}

  bool start() { return HAL_TIM_PWM_Start(handle_, channel_) == HAL_OK; }

  bool stop() { return HAL_TIM_PWM_Stop(handle_, channel_) == HAL_OK; }

  uint32_t get_compare() const {
    return __HAL_TIM_GET_COMPARE(handle_, channel_);
  }

  void set_compare(uint32_t compare) {
    __HAL_TIM_SET_COMPARE(handle_, channel_, compare);
  }

private:
  TIM_HandleTypeDef *handle_;
  uint32_t channel_;
};

#endif

} // namespace halx::peripheral