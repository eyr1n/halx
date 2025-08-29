#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

#include <cmsis_os2.h>

#include "halx/core.hpp"

namespace halx::rtos {

class Timer {
private:
  struct Deleter {
    void operator()(osTimerId_t timer_id) { osTimerDelete(timer_id); }
  };

  using TimerId = std::unique_ptr<std::remove_pointer_t<osTimerId_t>, Deleter>;

public:
  Timer(void (*func)(void *), void *args, osTimerType_t type,
        uint32_t attr_bits = 0) {
    osTimerAttr_t attr{};
    attr.attr_bits = attr_bits;
    timer_id_ = TimerId{osTimerNew(func, type, args, &attr)};
  }

  Timer(std::move_only_function<void()> &&func, osTimerType_t type,
        uint32_t attr_bits = 0)
      : func_{std::move(func)} {
    osTimerAttr_t attr{};
    attr.attr_bits = attr_bits;
    timer_id_ = TimerId{
        osTimerNew(&core::Function<void()>::call, type, func_.c_ptr(), &attr)};
  }

  bool start(uint32_t ticks) {
    return osTimerStart(timer_id_.get(), ticks) == osOK;
  }

  bool stop() { return osTimerStop(timer_id_.get()) == osOK; }

  bool is_running() { return osTimerIsRunning(timer_id_.get()) == 1; }

private:
  TimerId timer_id_;
  core::Function<void()> func_;
};

} // namespace halx::rtos
