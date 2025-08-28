#pragma once

#include <functional>
#include <memory>

#include "halx/core.hpp"

extern "C" {
void EXTI0_IRQHandler();
void EXTI1_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void EXTI4_IRQHandler();
void EXTI9_5_IRQHandler();
void EXTI15_10_IRQHandler();
}

namespace halx::peripheral {

class ExtiBase {
public:
  virtual ~ExtiBase() {}
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

#ifdef HAL_EXTI_MODULE_ENABLED

template <uint32_t Line> class Exti : public ExtiBase {
private:
  struct State {
    EXTI_HandleTypeDef hexti{};
    void (*callback)(void *context) = nullptr;
    void *context = nullptr;

    State(uint32_t mode, uint32_t trigger, uint32_t gpio_sel,
          uint32_t preempt_priority, uint32_t sub_priority) {
      set_context(this);
      HAL_EXTI_RegisterCallback(&hexti, HAL_EXTI_COMMON_CB_ID, [] {
        auto state = get_context();
        if (state->callback) {
          state->callback(state->context);
        }
      });
      EXTI_ConfigTypeDef config{
          .Line = Line,
          .Mode = mode,
          .Trigger = trigger,
          .GPIOSel = gpio_sel,
      };
      HAL_EXTI_SetConfigLine(&hexti, &config);
      IRQn_Type irqn = get_irqn(Line);
      HAL_NVIC_SetPriority(irqn, preempt_priority, sub_priority);
      HAL_NVIC_EnableIRQ(irqn);
    }

    ~State() {
      IRQn_Type irqn = get_irqn(Line);
      HAL_NVIC_DisableIRQ(irqn);
      HAL_EXTI_ClearConfigLine(&hexti);
      HAL_EXTI_RegisterCallback(&hexti, HAL_EXTI_COMMON_CB_ID, nullptr);
      set_context(nullptr);
    }

    static inline IRQn_Type get_irqn(uint32_t line) {
      switch (line) {
      case EXTI_LINE_0:
        return EXTI0_IRQn;
      case EXTI_LINE_1:
        return EXTI1_IRQn;
      case EXTI_LINE_2:
        return EXTI2_IRQn;
      case EXTI_LINE_3:
        return EXTI3_IRQn;
      case EXTI_LINE_4:
        return EXTI4_IRQn;
      case EXTI_LINE_5:
      case EXTI_LINE_6:
      case EXTI_LINE_7:
      case EXTI_LINE_8:
      case EXTI_LINE_9:
        return EXTI9_5_IRQn;
      case EXTI_LINE_10:
      case EXTI_LINE_11:
      case EXTI_LINE_12:
      case EXTI_LINE_13:
      case EXTI_LINE_14:
      case EXTI_LINE_15:
        return EXTI15_10_IRQn;
      }
      std::unreachable();
    }
  };

public:
  using ExtiBase::attach_callback;

  Exti(uint32_t mode, uint32_t trigger, uint32_t gpio_sel,
       uint32_t preempt_priority, uint32_t sub_priority)
      : state_{std::make_unique<State>(mode, trigger, gpio_sel,
                                       preempt_priority, sub_priority)} {}

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

  friend void ::EXTI0_IRQHandler();
  friend void ::EXTI1_IRQHandler();
  friend void ::EXTI2_IRQHandler();
  friend void ::EXTI3_IRQHandler();
  friend void ::EXTI4_IRQHandler();
  friend void ::EXTI9_5_IRQHandler();
  friend void ::EXTI15_10_IRQHandler();
};

#endif

} // namespace halx::peripheral