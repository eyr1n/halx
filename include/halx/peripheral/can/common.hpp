#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "halx/core.hpp"

namespace halx::peripheral {

struct CanFilter {
  uint32_t id;
  uint32_t mask;
  bool ide;
};

struct CanMessage {
  uint32_t id;
  bool ide;
  uint8_t dlc;
  std::array<uint8_t, 8> data;
};

class CanBase {
public:
  virtual ~CanBase() {}
  virtual bool start() = 0;
  virtual bool stop() = 0;
  virtual bool transmit(const CanMessage &msg, uint32_t timeout) = 0;
  virtual std::optional<size_t>
  attach_rx_filter(const CanFilter &filter,
                   void (*callback)(const CanMessage &msg, void *context),
                   void *context) = 0;
  virtual bool detach_rx_filter(size_t filter_index) = 0;

  template <class T>
  std::optional<size_t> attach_rx_queue(const CanFilter &filter, T &queue) {
    return attach_rx_filter(
        filter,
        [](const CanMessage &msg, void *context) {
          auto *queue = reinterpret_cast<T *>(context);
          queue->push(msg);
        },
        &queue);
  }
};

} // namespace halx::peripheral
