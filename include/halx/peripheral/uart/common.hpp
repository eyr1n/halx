#pragma once

#include <cstddef>
#include <cstdint>

namespace halx::peripheral {

/**
 * デフォルトでTx, Rxともに割り込みを使用します。
 *
 * @code{.cpp}
 * #include <cstdio>
 * #include <halx/core.hpp>
 * #include <halx/peripheral.hpp>
 *
 * extern UART_HandleTypeDef huart2;
 *
 * extern "C" void main_thread(void *) {
 *   using namespace halx::core;
 *   using namespace halx::peripheral;
 *
 *   Uart<&huart2> uart2;
 *   enable_stdout(uart2);
 *
 *   while (true) {
 *     // 7バイト送信
 *     uint8_t data[] = {'h', 'e', 'l', 'l', 'o', '\r', '\n'};
 *     uart2.transmit(data, sizeof(data), MAX_DELAY);
 *
 *     // 1バイト受信
 *     char c;
 *     if (uart2.receive((uint8_t *)&c, 1, MAX_DELAY)) {
 *       printf("入力した文字: %c\r\n", c);
 *     }
 *
 *     delay(10);
 *   }
 * }
 * @endcode
 */
class UartBase {
public:
  virtual ~UartBase() {}
  virtual bool transmit(const uint8_t *data, size_t size, uint32_t timeout) = 0;
  virtual bool receive(uint8_t *data, size_t size, uint32_t timeout) = 0;
  virtual void flush() = 0;
  virtual size_t available() const = 0;
};

bool enable_stdout(UartBase &uart);

bool disable_stdout();

} // namespace halx::peripheral
