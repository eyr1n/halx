#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "halx/core.hpp"

#include "common.hpp"

namespace halx::peripheral {

template <UART_HandleTypeDef *Handle> class UartTxDma {
private:
  struct State {
    core::Notifier notifier;

    State() {
      stm32cubemx_helper::set_context<Handle, State>(this);
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_TX_COMPLETE_CB_ID, [](UART_HandleTypeDef *) {
            auto state = stm32cubemx_helper::get_context<Handle, State>();
            state->notifier.set(0x1);
          });
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_ERROR_CB_ID, [](UART_HandleTypeDef *) {
            auto state = stm32cubemx_helper::get_context<Handle, State>();
            state->notifier.set(0x2);
          });
    }
  };

  struct Deleter {
    void operator()(State *state) {
      HAL_UART_AbortTransmit(Handle);
      HAL_UART_UnRegisterCallback(Handle, HAL_UART_TX_COMPLETE_CB_ID);
      HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
      stm32cubemx_helper::set_context<Handle, State>(nullptr);
      delete state;
    }
  };

public:
  UartTxDma() : state_{new State{}} {}

  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    state_->notifier.reset();
    if (HAL_UART_Transmit_DMA(Handle, data, size) != HAL_OK) {
      HAL_UART_AbortTransmit(Handle);
      return false;
    }
    if (state_->notifier.wait(0x1 | 0x2, timeout) != 0x1) {
      HAL_UART_AbortTransmit(Handle);
      return false;
    }
    return true;
  }

private:
  std::unique_ptr<State, Deleter> state_;
};

template <UART_HandleTypeDef *Handle> class UartRxDma {
private:
  struct State {
    uint8_t *buf;
    size_t buf_size;
    std::atomic<size_t> read_idx{0};

    State(uint8_t *buf, size_t buf_size) : buf{buf}, buf_size{buf_size} {
      stm32cubemx_helper::set_context<Handle, State>(this);
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_ERROR_CB_ID,
          [](UART_HandleTypeDef *huart) { HAL_UART_AbortReceive_IT(huart); });
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID,
          [](UART_HandleTypeDef *huart) {
            auto state = stm32cubemx_helper::get_context<Handle, State>();
            HAL_UART_Receive_DMA(huart, state->buf, state->buf_size);
            state->read_idx.store(0, std::memory_order_relaxed);
          });
      HAL_UART_Receive_DMA(Handle, buf, buf_size);
    }
  };

  struct Deleter {
    void operator()(State *state) {
      HAL_UART_AbortReceive(Handle);
      HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
      HAL_UART_UnRegisterCallback(Handle,
                                  HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID);
      stm32cubemx_helper::set_context<Handle, State>(nullptr);
      delete state;
    }
  };

public:
  UartRxDma(uint8_t *buf, size_t buf_size) : state_{new State{buf, buf_size}} {}

  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    core::TimeoutHelper timeout_helper{timeout};
    while (available() < size) {
      if (timeout_helper.is_timeout()) {
        return false;
      }
      core::yield();
    }
    for (size_t i = 0; i < size; ++i) {
      data[i] = state_->buf[state_->read_idx.load(std::memory_order_relaxed)];
      advance(1);
    }
    return true;
  }

  void flush() { advance(available()); }

  size_t available() const {
    size_t write_idx = state_->buf_size - __HAL_DMA_GET_COUNTER(Handle->hdmarx);
    size_t read_idx = state_->read_idx.load(std::memory_order_relaxed);
    return (state_->buf_size + write_idx - read_idx) % state_->buf_size;
  }

private:
  std::unique_ptr<State, Deleter> state_;

  void advance(size_t len) {
    size_t read_idx = state_->read_idx.load(std::memory_order_relaxed);
    while (true) {
      if (state_->read_idx.compare_exchange_weak(
              read_idx, (read_idx + len) % state_->buf_size,
              std::memory_order_relaxed, std::memory_order_relaxed)) {
        break;
      }
    }
  }
};

} // namespace halx::peripheral
