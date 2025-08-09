#pragma once

#include "uart/common.hpp"

#ifdef HAL_UART_MODULE_ENABLED
#include "uart/uart_dma.hpp"
#include "uart/uart_it.hpp"
#include "uart/uart_poll.hpp"
#endif

namespace halx::peripheral {

#ifdef HAL_UART_MODULE_ENABLED

template <UART_HandleTypeDef *Handle, UartType TxType> class UartTx;

template <UART_HandleTypeDef *Handle> class UartTx<Handle, UartType::POLL> {
public:
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return tx_.transmit(data, size, timeout);
  }

private:
  UartTxPoll<Handle> tx_;
};

template <UART_HandleTypeDef *Handle> class UartTx<Handle, UartType::IT> {
public:
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return tx_.transmit(data, size, timeout);
  }

private:
  UartTxIt<Handle> tx_;
};

template <UART_HandleTypeDef *Handle> class UartTx<Handle, UartType::DMA> {
public:
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return tx_.transmit(data, size, timeout);
  }

private:
  UartTxDma<Handle> tx_;
};

template <UART_HandleTypeDef *Handle, UartType RxType> class UartRx;

template <UART_HandleTypeDef *Handle> class UartRx<Handle, UartType::POLL> {
public:
  UartRx(size_t buf_size) : rx_{buf_size} {}
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return rx_.receive(data, size, timeout);
  }
  void flush() { rx_.flush(); }
  size_t available() const { return rx_.available(); }

private:
  UartRxPoll<Handle> rx_;
};

template <UART_HandleTypeDef *Handle> class UartRx<Handle, UartType::IT> {
public:
  UartRx(size_t buf_size) : rx_{buf_size} {}
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return rx_.receive(data, size, timeout);
  }
  void flush() { rx_.flush(); }
  size_t available() const { return rx_.available(); }

private:
  UartRxIt<Handle> rx_;
};

template <UART_HandleTypeDef *Handle> class UartRx<Handle, UartType::DMA> {
public:
  UartRx(size_t buf_size) : rx_{buf_size} {}
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return rx_.receive(data, size, timeout);
  }
  void flush() { rx_.flush(); }
  size_t available() const { return rx_.available(); }

private:
  UartRxDma<Handle> rx_;
};

template <UART_HandleTypeDef *Handle, UartType TxType = UartType::IT,
          UartType RxType = UartType::IT>
class Uart : public UartBase {
public:
  Uart(size_t rx_buf_size = 64) : rx_{rx_buf_size} {}
  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    return tx_.transmit(data, size, timeout);
  }
  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    return rx_.receive(data, size, timeout);
  }
  void flush() { rx_.flush(); }
  size_t available() const { return rx_.available(); }

private:
  UartTx<Handle, TxType> tx_;
  UartRx<Handle, RxType> rx_;
};

#endif

} // namespace halx::peripheral