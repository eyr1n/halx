#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "halx/core.hpp"

#include "common.hpp"

namespace halx::peripheral {

template <UART_HandleTypeDef *Handle> class UartTxIt {
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
  UartTxIt() : state_{new State{}} {}

  bool transmit(const uint8_t *data, size_t size, uint32_t timeout) {
    state_->notifier.reset();
    if (HAL_UART_Transmit_IT(Handle, data, size) != HAL_OK) {
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

template <UART_HandleTypeDef *Handle> class UartRxIt {
private:
  struct State {
    core::RingBuffer<uint8_t> queue;
    uint8_t buf;

    State(size_t buf_size) : queue{buf_size} {
      stm32cubemx_helper::set_context<Handle, State>(this);
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_RX_COMPLETE_CB_ID, [](UART_HandleTypeDef *huart) {
            auto state = stm32cubemx_helper::get_context<Handle, State>();
            state->queue.push(state->buf);
            HAL_UART_Receive_IT(huart, &state->buf, 1);
          });
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_ERROR_CB_ID,
          [](UART_HandleTypeDef *huart) { HAL_UART_AbortReceive_IT(huart); });
      HAL_UART_RegisterCallback(
          Handle, HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID,
          [](UART_HandleTypeDef *huart) {
            auto state = stm32cubemx_helper::get_context<Handle, State>();
            HAL_UART_Receive_IT(huart, &state->buf, 1);
          });
      HAL_UART_Receive_IT(Handle, &buf, 1);
    }
  };

  struct Deleter {
    void operator()(State *state) {
      HAL_UART_AbortReceive(Handle);
      HAL_UART_UnRegisterCallback(Handle, HAL_UART_RX_COMPLETE_CB_ID);
      HAL_UART_UnRegisterCallback(Handle, HAL_UART_ERROR_CB_ID);
      HAL_UART_UnRegisterCallback(Handle,
                                  HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID);
      stm32cubemx_helper::set_context<Handle, State>(nullptr);
      delete state;
    }
  };

public:
  UartRxIt(size_t buf_size) : state_{new State{buf_size}} {}

  bool receive(uint8_t *data, size_t size, uint32_t timeout) {
    core::TimeoutHelper timeout_helper{timeout};
    while (state_->queue.size() < size) {
      if (timeout_helper.is_timeout()) {
        return false;
      }
      core::yield();
    }
    for (size_t i = 0; i < size; ++i) {
      data[i] = *state_->queue.pop();
    }
    return true;
  }

  void flush() { state_->queue.clear(); }

  size_t available() const { return state_->queue.size(); }

private:
  std::unique_ptr<State, Deleter> state_;
};

} // namespace halx::peripheral
