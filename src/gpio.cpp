#include "halx/peripheral/gpio.hpp"
#include "halx/core.hpp"

#ifdef HAL_GPIO_MODULE_ENABLED

#define GENERATE_CALLBACK_CASE(pin)                                            \
  case pin: {                                                                  \
    auto state = halx::peripheral::Gpio<pin>::get_context();                   \
    if (state && state->callback) {                                            \
      state->callback(state->context);                                         \
    }                                                                          \
    break;                                                                     \
  }

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  switch (GPIO_Pin) {
    GENERATE_CALLBACK_CASE(GPIO_PIN_0)
    GENERATE_CALLBACK_CASE(GPIO_PIN_1)
    GENERATE_CALLBACK_CASE(GPIO_PIN_2)
    GENERATE_CALLBACK_CASE(GPIO_PIN_3)
    GENERATE_CALLBACK_CASE(GPIO_PIN_4)
    GENERATE_CALLBACK_CASE(GPIO_PIN_5)
    GENERATE_CALLBACK_CASE(GPIO_PIN_6)
    GENERATE_CALLBACK_CASE(GPIO_PIN_7)
    GENERATE_CALLBACK_CASE(GPIO_PIN_8)
    GENERATE_CALLBACK_CASE(GPIO_PIN_9)
    GENERATE_CALLBACK_CASE(GPIO_PIN_10)
    GENERATE_CALLBACK_CASE(GPIO_PIN_11)
    GENERATE_CALLBACK_CASE(GPIO_PIN_12)
    GENERATE_CALLBACK_CASE(GPIO_PIN_13)
    GENERATE_CALLBACK_CASE(GPIO_PIN_14)
    GENERATE_CALLBACK_CASE(GPIO_PIN_15)
  default:
    std::unreachable();
  }
}

#endif // HAL_GPIO_MODULE_ENABLED