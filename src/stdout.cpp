#include "halx/core.hpp"
#include "halx/peripheral/uart.hpp"

namespace halx::peripheral {

static UartBase **uart_stdout() {
  static UartBase *uart;
  return &uart;
}

bool enable_stdout(UartBase &uart) {
  if (*uart_stdout()) {
    return false;
  }
  *uart_stdout() = &uart;
  return true;
}

bool disable_stdout() {
  if (!*uart_stdout()) {
    return false;
  }
  *uart_stdout() = nullptr;
  return true;
}

} // namespace halx::peripheral

extern "C" int _write(int, char *ptr, int len) {
  auto uart = *halx::peripheral::uart_stdout();
  if (uart) {
    if (uart->transmit(reinterpret_cast<uint8_t *>(ptr), len,
                       halx::core::MAX_DELAY)) {
      return len;
    }
  }
  return -1;
}
