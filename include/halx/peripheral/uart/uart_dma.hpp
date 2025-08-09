#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "halx/core.hpp"

#include "common.hpp"

namespace halx::peripheral {

template <UART_HandleTypeDef *Handle> class UartTxDma {
public:
  UartTxDma() {
    stm32cubemx_helper::set_context<Handle, UartTxDma>(this);
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_TX_COMPLETE_CB_ID, [](UART_HandleTypeDef *) {
          auto tx = stm32cubemx_helper::get_context<Handle, UartTxDma>();
          tx->notifier_.set(0x1);
        });
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_ERROR_CB_ID, [](UART_HandleTypeDef *) {
          auto tx = stm32cubemx_helper::get_context<Handle, UartTxDma>();
          tx->notifier_.set(0x2);
        });
  };

  ~UartTxDma() {
    HAL_UART_AbortTransmit(Handle);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_TX_COMPLETE_CB_ID);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
    stm32cubemx_helper::set_context<Handle, UartTxDma>(nullptr);
  }

  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    notifier_.reset();
    if (HAL_UART_Transmit_DMA(Handle, data, size) != HAL_OK) {
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

  UartTxDma(const UartTxDma &) = delete;
  UartTxDma &operator=(const UartTxDma &) = delete;
};

template <UART_HandleTypeDef *Handle> class UartRxDma {
public:
  UartRxDma(size_t buf_size = 64) : buf_(buf_size) {
    stm32cubemx_helper::set_context<Handle, UartRxDma>(this);
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_ERROR_CB_ID,
        [](UART_HandleTypeDef *huart) { HAL_UART_AbortReceive_IT(huart); });
    HAL_UART_RegisterCallback(
        Handle, HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID,
        [](UART_HandleTypeDef *huart) {
          auto rx = stm32cubemx_helper::get_context<Handle, UartRxDma>();
          HAL_UART_Receive_DMA(huart, rx->buf_.data(), rx->buf_.size());
          rx->read_idx_.store(0, std::memory_order_relaxed);
        });
    HAL_UART_Receive_DMA(Handle, buf_.data(), buf_.size());
  }

  ~UartRxDma() {
    HAL_UART_AbortReceive(Handle);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
    HAL_UART_UnRegisterCallback(Handle, HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID);
    stm32cubemx_helper::set_context<Handle, UartRxDma>(nullptr);
  }

  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    core::TimeoutHelper timeout_helper{timeout};
    while (available() < size) {
      if (timeout_helper.is_timeout()) {
        return false;
      }
      core::yield();
    }
    for (size_t i = 0; i < size; ++i) {
      data[i] = buf_[read_idx_];
      advance(1);
    }
    return true;
  }

  void flush() { advance(available()); }

  size_t available() const {
    size_t write_idx = buf_.size() - __HAL_DMA_GET_COUNTER(Handle->hdmarx);
    size_t read_idx = read_idx_.load(std::memory_order_relaxed);
    return (buf_.size() + write_idx - read_idx) % buf_.size();
  }

private:
  std::vector<uint8_t> buf_;
  std::atomic<size_t> read_idx_{0};

  UartRxDma(const UartRxDma &) = delete;
  UartRxDma &operator=(const UartRxDma &) = delete;

  void advance(size_t len) {
    size_t read_idx = read_idx_.load(std::memory_order_relaxed);
    while (true) {
      if (read_idx_.compare_exchange_weak(
              read_idx, (read_idx + len) % buf_.size(),
              std::memory_order_relaxed, std::memory_order_relaxed)) {
        break;
      }
    }
  }
};

} // namespace halx::peripheral
