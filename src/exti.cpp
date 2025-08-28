#include "halx/peripheral/exti.hpp"
#include "halx/core.hpp"

void EXTI0_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_0>::hexti);
}

void EXTI1_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_1>::hexti);
}

void EXTI2_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_2>::hexti);
}

void EXTI3_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_3>::hexti);
}

void EXTI4_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_4>::hexti);
}

void EXTI9_5_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_5>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_6>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_7>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_8>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_9>::hexti);
}

void EXTI15_10_IRQHandler() {
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_10>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_11>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_12>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_13>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_14>::hexti);
  HAL_EXTI_IRQHandler(&halx::peripheral::Exti<EXTI_LINE_15>::hexti);
}
