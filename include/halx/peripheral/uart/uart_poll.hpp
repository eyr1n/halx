#pragma once

#include <cstddef>
#include <cstdint>

#include "halx/core.hpp"

#include "common.hpp"

namespace halx::peripheral {

template <UART_HandleTypeDef *Handle> class UartTxPoll {
public:
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return HAL_UART_Transmit(Handle, data, size, timeout) == HAL_OK;
  }
};

template <UART_HandleTypeDef *Handle> class UartRxPoll {
public:
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return HAL_UART_Receive(Handle, data, size, timeout);
  }

  void flush() {}

  size_t available() const { return 0; }
};

} // namespace halx::peripheral
