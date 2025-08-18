#pragma once

#include "can/common.hpp"

#ifdef HAL_CAN_MODULE_ENABLED
#include "can/bxcan.hpp"
#endif

#ifdef HAL_FDCAN_MODULE_ENABLED
#include "can/fdcan.hpp"
#endif

namespace halx::peripheral {

template <auto *Handle, class HandleType = decltype(Handle)> class Can;

#ifdef HAL_CAN_MODULE_ENABLED

template <auto *Handle>
class Can<Handle, CAN_HandleTypeDef *> : public CanBase {
public:
  using CanBase::attach_rx_filter;

  bool start() override { return can_.start(); }
  bool stop() override { return can_.stop(); }
  bool transmit(const CanMessage &msg, uint32_t timeout) override {
    return can_.transmit(msg, timeout);
  }
  std::optional<size_t>
  attach_rx_filter(const CanFilter &filter,
                   void (*callback)(void *context, const CanMessage &msg),
                   void *context) override {
    return can_.attach_rx_filter(filter, callback, context);
  }
  bool detach_rx_filter(size_t filter_index) override {
    return can_.detach_rx_filter(filter_index);
  }

private:
  BxCan<Handle> can_;
};

#endif

#ifdef HAL_FDCAN_MODULE_ENABLED

template <auto *Handle>
class Can<Handle, FDCAN_HandleTypeDef *> : public CanBase {
public:
  using CanBase::attach_rx_filter;

  bool start() override { return can_.start(); }
  bool stop() override { return can_.stop(); }
  bool transmit(const CanMessage &msg, uint32_t timeout) override {
    return can_.transmit(msg, timeout);
  }
  std::optional<size_t>
  attach_rx_filter(const CanFilter &filter,
                   void (*callback)(void *context, const CanMessage &msg),
                   void *context) override {
    return can_.attach_rx_filter(filter, callback, context);
  }
  bool detach_rx_filter(size_t filter_index) override {
    return can_.detach_rx_filter(filter_index);
  }

private:
  FdCan<Handle> can_;
};

#endif

} // namespace halx::peripheral