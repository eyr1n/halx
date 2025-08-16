#pragma once

#include "uart/common.hpp"

#ifdef HAL_UART_MODULE_ENABLED
#include "uart/uart_dma.hpp"
#include "uart/uart_it.hpp"
#include "uart/uart_poll.hpp"
#endif

namespace halx::peripheral {

#ifdef HAL_UART_MODULE_ENABLED

template <class Tx, class Rx> class Uart : public UartBase {
public:
  Uart(Tx &&tx, Rx &&rx) : tx_{std::move(tx)}, rx_{std::move(rx)} {}
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return tx_.transmit(data, size, timeout);
  }
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return rx_.receive(data, size, timeout);
  }
  void flush() { rx_.flush(); }
  size_t available() const { return rx_.available(); }

private:
  Tx tx_;
  Rx rx_;
};

#endif

} // namespace halx::peripheral