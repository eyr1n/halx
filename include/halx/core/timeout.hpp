#pragma once

#include <cstdint>

#include "common.hpp"

namespace halx::core {

class Timeout {
public:
  Timeout(uint32_t timeout)
      : deadline_{get_tick() + timeout}, no_timeout_{timeout == MAX_DELAY} {}

  operator bool() const {
    return !no_timeout_ && static_cast<int32_t>(get_tick() - deadline_) >= 0;
  }

private:
  uint32_t deadline_;
  bool no_timeout_;
};

} // namespace halx::core
