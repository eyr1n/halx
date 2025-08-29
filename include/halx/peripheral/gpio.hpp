#pragma once

#include <cstdint>

#include "halx/core.hpp"

namespace halx::peripheral {

#ifdef HAL_GPIO_MODULE_ENABLED

class Gpio {
public:
  Gpio(GPIO_TypeDef *port, uint16_t pin) : port_{port}, pin_{pin} {}

  uint8_t read() const {
    return HAL_GPIO_ReadPin(port_, pin_) == GPIO_PIN_SET ? 1 : 0;
  }

  void write(uint8_t state) {
    HAL_GPIO_WritePin(port_, pin_, state == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }

private:
  GPIO_TypeDef *port_;
  uint16_t pin_;
};

#endif

} // namespace halx::peripheral