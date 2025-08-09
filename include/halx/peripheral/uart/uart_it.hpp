#pragma once

#include <cstddef>
#include <cstdint>

#include "halx/core.hpp"

#include "common.hpp"

namespace halx::peripheral {

template <UART_HandleTypeDef *Handle> class UartTxIt {
public:
  UartTxIt() {
    stm32cubemx_helper::set_context<Handle, UartTxIt>(this);
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_TX_COMPLETE_CB_ID, [](UART_HandleTypeDef *) {
          auto tx = stm32cubemx_helper::get_context<Handle, UartTxIt>();
          tx->notifier_.set(0x1);
        });
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_ERROR_CB_ID, [](UART_HandleTypeDef *) {
          auto tx = stm32cubemx_helper::get_context<Handle, UartTxIt>();
          tx->notifier_.set(0x2);
        });
  };

  ~UartTxIt() {
    HAL_UART_AbortTransmit(Handle);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_TX_COMPLETE_CB_ID);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
    stm32cubemx_helper::set_context<Handle, UartTxIt>(nullptr);
  }

  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    notifier_.reset();
    if (HAL_UART_Transmit_IT(Handle, data, size) != HAL_OK) {
      HAL_UART_AbortTransmit(Handle);
      return false;
    }
    if (notifier_.wait(0x1 | 0x2, timeout) != 0x1) {
      HAL_UART_AbortTransmit(Handle);
      return false;
    }
    return true;
  }

private:
  core::Notifier notifier_;

  UartTxIt(const UartTxIt &) = delete;
  UartTxIt &operator=(const UartTxIt &) = delete;
};

template <UART_HandleTypeDef *Handle> class UartRxIt {
public:
  UartRxIt(size_t buf_size) : queue_{buf_size} {
    stm32cubemx_helper::set_context<Handle, UartRxIt>(this);
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_RX_COMPLETE_CB_ID, [](UART_HandleTypeDef *huart) {
          auto uart = stm32cubemx_helper::get_context<Handle, UartRxIt>();
          uart->queue_.push(uart->buf_);
          HAL_UART_Receive_IT(huart, &uart->buf_, 1);
        });
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_ERROR_CB_ID,
        [](UART_HandleTypeDef *huart) { HAL_UART_AbortReceive_IT(huart); });
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID,
        [](UART_HandleTypeDef *huart) {
          auto rx = stm32cubemx_helper::get_context<Handle, UartRxIt>();
          HAL_UART_Receive_IT(huart, &rx->buf_, 1);
        });
    HAL_UART_Receive_IT(Handle, &buf_, 1);
  }

  ~UartRxIt() {
    HAL_UART_AbortReceive(Handle);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_RX_COMPLETE_CB_ID);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID);
    stm32cubemx_helper::set_context<Handle, UartRxIt>(nullptr);
  }

  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    core::TimeoutHelper timeout_helper{timeout};
    while (queue_.size() < size) {
      if (timeout_helper.is_timeout()) {
        return false;
      }
      core::yield();
    }
    for (size_t i = 0; i < size; ++i) {
      data[i] = *queue_.pop();
    }
    return true;
  }

  void flush() { queue_.clear(); }

  size_t available() const { return queue_.size(); }

private:
  core::RingBuffer<uint8_t> queue_;
  uint8_t buf_;

  UartRxIt(const UartRxIt &) = delete;
  UartRxIt &operator=(const UartRxIt &) = delete;
};

} // namespace halx::peripheral
