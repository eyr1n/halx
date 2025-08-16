#pragma once

#include <array>
#include <cstdint>
#include <functional>
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

/**
 * @code{.cpp}
 * #include <cstdio>
 * #include <halx/core.hpp>
 * #include <halx/peripheral.hpp>
 *
 * extern UART_HandleTypeDef huart2;
 * extern FDCAN_HandleTypeDef hfdcan1;
 *
 * extern "C" void main_thread(void *) {
 *   using namespace halx::core;
 *   using namespace halx::peripheral;
 *
 *   Uart<&huart2> uart2;
 *   enable_stdout(uart2);
 *
 *   Can<&hfdcan1> can1;
 *
 *   // 受信フィルター、受信キューの設定
 *   CanFilter rx_filter = {
 *       .id = 0x0,
 *       .mask = 0x0,
 *       .ide = false,
 *   };
 *   RingBuffer<CanMessage> rx_queue(10);
 *   can1.attach_rx_queue(rx_filter, rx_queue);
 *
 *   // CAN通信開始
 *   can1.start();
 *
 *   // 送信するメッセージの作成
 *   CanMessage tx_message = {
 *       .id = 0x3,
 *       .ide = false,
 *       .dlc = 1,
 *       .data = {0x0},
 *   };
 *
 *   for (int i = 0; i < 10; ++i) {
 *     // 送信
 *     can1.transmit(tx_message, MAX_DELAY);
 *     tx_message.data[0]++;
 *   }
 *
 *   while (true) {
 *     // 受信
 *     CanMessage rx_message;
 *     if (rx_queue.pop(rx_message, MAX_DELAY)) {
 *       printf("id: %d, data: %d\r\n", (int)rx_message.id,
 *              (int)rx_message.data[0]);
 *     }
 *     delay(10);
 *   }
 * }
 * @endcode
 */
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

  std::optional<size_t>
  attach_rx_filter(const CanFilter &filter,
                   std::function<void(const CanMessage &)> &&callback) {
    callback_ = std::move(callback);
    return attach_rx_filter(
        filter,
        [](const CanMessage &msg, void *context) {
          auto callback =
              reinterpret_cast<std::function<void(const CanMessage &)> *>(
                  context);
          (*callback)(msg);
        },
        &callback_);
  }

  template <class Queue>
  std::optional<size_t> attach_rx_queue(const CanFilter &filter, Queue &queue) {
    return attach_rx_filter(
        filter,
        [](const CanMessage &msg, void *context) {
          auto *queue = reinterpret_cast<Queue *>(context);
          queue->push(msg);
        },
        &queue);
  }

private:
  std::function<void(const CanMessage &)> callback_;
};

} // namespace halx::peripheral
