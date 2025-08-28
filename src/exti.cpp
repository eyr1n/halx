#include "halx/peripheral/exti.hpp"
#include "halx/core.hpp"

void EXTI0_IRQHandler() {
  auto state = halx::peripheral::Exti<EXTI_LINE_0>::get_context();
  if (state) {
    HAL_EXTI_IRQHandler(&state->hexti);
  }
}

void EXTI1_IRQHandler() {
  auto state = halx::peripheral::Exti<EXTI_LINE_1>::get_context();
  if (state) {
    HAL_EXTI_IRQHandler(&state->hexti);
  }
}

void EXTI2_IRQHandler() {
  auto state = halx::peripheral::Exti<EXTI_LINE_2>::get_context();
  if (state) {
    HAL_EXTI_IRQHandler(&state->hexti);
  }
}

void EXTI3_IRQHandler() {
  auto state = halx::peripheral::Exti<EXTI_LINE_3>::get_context();
  if (state) {
    HAL_EXTI_IRQHandler(&state->hexti);
  }
}

void EXTI4_IRQHandler() {
  auto state = halx::peripheral::Exti<EXTI_LINE_4>::get_context();
  if (state) {
    HAL_EXTI_IRQHandler(&state->hexti);
  }
}

void EXTI9_5_IRQHandler() {
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_5>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_6>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_7>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_8>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_9>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
}

void EXTI15_10_IRQHandler() {
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_10>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_11>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_12>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_13>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_14>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
  {
    auto state = halx::peripheral::Exti<EXTI_LINE_15>::get_context();
    if (state) {
      HAL_EXTI_IRQHandler(&state->hexti);
    }
  }
}
