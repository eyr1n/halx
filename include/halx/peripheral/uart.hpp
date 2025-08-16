#pragma once

#include "uart/common.hpp"

#ifdef HAL_UART_MODULE_ENABLED
#include "uart/uart_dma.hpp"
#include "uart/uart_it.hpp"
#include "uart/uart_poll.hpp"
#endif

namespace halx::peripheral {

#ifdef HAL_UART_MODULE_ENABLED

template <UART_HandleTypeDef *Handle,
          template <UART_HandleTypeDef *> class UartTx = UartTxIt,
          template <UART_HandleTypeDef *> class UartRx = UartRxIt>
class Uart : public UartBase {
public:
  Uart(UartTx<Handle> &&tx = {}, UartRx<Handle> &&rx = {})
      : tx_{std::move(tx)}, rx_{std::move(rx)} {}
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return tx_.transmit(data, size, timeout);
  }
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return rx_.receive(data, size, timeout);
  }
  void flush() { rx_.flush(); }
  size_t available() const { return rx_.available(); }

private:
  UartTx<Handle> tx_;
  UartRx<Handle> rx_;
};

#endif

} // namespace halx::peripheral