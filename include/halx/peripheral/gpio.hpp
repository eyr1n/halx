#pragma once

#include <functional>
#include <memory>

#include "halx/core.hpp"

namespace halx::peripheral {

class GpioBase {
public:
  virtual ~GpioBase() {}
  virtual uint8_t read() = 0;
  virtual void write(uint8_t value) = 0;
  virtual void toggle() = 0;
  virtual bool attach_callback(void (*callback)(void *context),
                               void *context) = 0;
  virtual bool detach_callback() = 0;

  bool attach_callback(std::move_only_function<void()> &&callback) {
    callback_ = std::move(callback);
    return attach_callback(callback_.call, callback_.c_ptr());
  }

private:
  core::Function<void()> callback_;
};

#ifdef HAL_GPIO_MODULE_ENABLED

template <uint16_t Pin> class Gpio : public GpioBase {
private:
  struct State {
    GPIO_TypeDef *port;
    void (*callback)(void *context) = nullptr;
    void *context = nullptr;

    State(GPIO_TypeDef *port) : port{port} { set_context(this); }

    ~State() { set_context(nullptr); }
  };

public:
  using GpioBase::attach_callback;

  Gpio(GPIO_TypeDef *port) : state_{std::make_unique<State>(port)} {}

  uint8_t read() override {
    return HAL_GPIO_ReadPin(state_->port, Pin) == GPIO_PIN_SET ? 1 : 0;
  }

  void write(uint8_t value) override {
    HAL_GPIO_WritePin(state_->port, Pin,
                      value == 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }

  void toggle() override { HAL_GPIO_TogglePin(state_->port, Pin); }

  bool attach_callback(void (*callback)(void *context),
                       void *context) override {
    if (state_->callback) {
      return false;
    }
    state_->callback = callback;
    state_->context = context;
    return true;
  }

  bool detach_callback() override {
    if (!state_->callback) {
      return false;
    }
    state_->callback = nullptr;
    return true;
  }

private:
  std::unique_ptr<State> state_;

  static inline State *&context() {
    static State *context = nullptr;
    return context;
  }

  static inline State *get_context() { return context(); }

  static inline void set_context(State *value) { context() = value; }

  friend void ::HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
};

#endif

} // namespace halx::peripheral