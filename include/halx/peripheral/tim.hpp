#pragma once

#include <functional>

#include "halx/core.hpp"

namespace halx::peripheral {

class TimBase {
public:
  virtual ~TimBase() {}
  virtual bool start() = 0;
  virtual bool stop() = 0;
  virtual uint32_t get_counter() const = 0;
  virtual void set_counter(uint32_t count) = 0;
  virtual bool attach_callback(void (*callback)(void *context),
                               void *context) = 0;
  virtual bool detach_callback() = 0;

  bool attach_callback(std::function<void()> &&callback) {
    if (callback_) {
      return false;
    }
    callback_ = std::move(callback);
    return attach_callback(
        [](void *context) {
          auto callback = reinterpret_cast<std::function<void()> *>(context);
          (*callback)();
        },
        &callback_);
  }

private:
  std::function<void()> callback_;
};

#ifdef HAL_TIM_MODULE_ENABLED

template <TIM_HandleTypeDef *Handle> class Tim : public TimBase {
public:
  Tim() {
    stm32cubemx_helper::set_context<Handle, Tim>(this);
    HAL_TIM_RegisterCallback(
        Handle, HAL_TIM_PERIOD_ELAPSED_CB_ID, [](TIM_HandleTypeDef *) {
          auto tim = stm32cubemx_helper::get_context<Handle, Tim>();
          if (tim->callback_) {
            tim->callback_(tim->context_);
          }
        });
  }

  ~Tim() override {
    HAL_TIM_UnRegisterCallback(Handle, HAL_TIM_PERIOD_ELAPSED_CB_ID);
    stm32cubemx_helper::set_context<Handle, Tim>(nullptr);
  }

  bool start() override { return HAL_TIM_Base_Start_IT(Handle) == HAL_OK; }

  bool stop() override { return HAL_TIM_Base_Stop_IT(Handle) == HAL_OK; }

  uint32_t get_counter() const override {
    return __HAL_TIM_GET_COUNTER(Handle);
  }

  void set_counter(uint32_t count) override {
    __HAL_TIM_SET_COUNTER(Handle, count);
  }

  bool attach_callback(void (*callback)(void *context),
                       void *context) override {
    if (callback) {
      return false;
    }
    callback_ = callback;
    context_ = context;
    return true;
  }

  bool detach_callback() override {
    if (!callback_) {
      return false;
    }
    callback_ = nullptr;
    return true;
  }

private:
  void (*callback_)(void *context);
  void *context_;
  std::function<void()> func_;
};

#endif

} // namespace halx::peripheral