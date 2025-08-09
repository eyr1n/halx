#pragma once

#include "can/common.hpp"

#ifdef HAL_CAN_MODULE_ENABLED
// #include "can/bxcan.hpp"
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
  bool start() override { return bxcan_.start(); }
  bool stop() override { return bxcan_.stop(); }
  bool transmit(const CanMessage &msg, uint32_t timeout) override {
    return bxcan_.transmit(msg, timeout);
  }
  std::optional<size_t> attach_rx_filter(const CanFilter &filter,
                                         void (*callback)(const CanMessage &msg,
                                                          void *context),
                                         void *context) override {
    return bxcan_.attach_rx_filter(filter, callback, context);
  }
  bool detach_rx_filter(size_t filter_index) override {
    return bxcan_.detach_rx_filter(filter_index);
  }

private:
  BxCan<Handle> bxcan_;
};

#endif

#ifdef HAL_FDCAN_MODULE_ENABLED

template <auto *Handle>
class Can<Handle, FDCAN_HandleTypeDef *> : public CanBase {
public:
  bool start() override { return fdcan_.start(); }
  bool stop() override { return fdcan_.stop(); }
  bool transmit(const CanMessage &msg, uint32_t timeout) override {
    return fdcan_.transmit(msg, timeout);
  }
  std::optional<size_t> attach_rx_filter(const CanFilter &filter,
                                         void (*callback)(const CanMessage &msg,
                                                          void *context),
                                         void *context) override {
    return fdcan_.attach_rx_filter(filter, callback, context);
  }
  bool detach_rx_filter(size_t filter_index) override {
    return fdcan_.detach_rx_filter(filter_index);
  }

private:
  FdCan<Handle> fdcan_;
};

#endif

} // namespace halx::peripheral