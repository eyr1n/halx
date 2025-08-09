#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <optional>
#include <vector>

#include "halx/core.hpp"

namespace halx::peripheral {

template <FDCAN_HandleTypeDef *Handle> class FdCan {
public:
  FdCan()
      : rx_callbacks_(Handle->Init.StdFiltersNbr + Handle->Init.ExtFiltersNbr,
                      nullptr),
        rx_callback_contexts_(
            Handle->Init.StdFiltersNbr + Handle->Init.ExtFiltersNbr, nullptr) {
    stm32cubemx_helper::set_context<Handle, FdCan>(this);
    HAL_FDCAN_RegisterRxFifo0Callback(
        Handle, [](FDCAN_HandleTypeDef *hfdcan, uint32_t) {
          FDCAN_RxHeaderTypeDef rx_header;
          CanMessage msg;

          auto fdcan = stm32cubemx_helper::get_context<Handle, FdCan>();

          while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header,
                                        msg.data.data()) == HAL_OK) {
            if (rx_header.IsFilterMatchingFrame == 1 ||
                rx_header.FilterIndex >= fdcan->rx_callbacks_.size()) {
              continue;
            }
            size_t filter_index = rx_header.FilterIndex;
            if (rx_header.IdType == FDCAN_EXTENDED_ID) {
              filter_index += Handle->Init.StdFiltersNbr;
            }
            auto rx_callback = fdcan->rx_callbacks_[filter_index];
            if (rx_callback) {
              update_rx_message(msg, rx_header);
              rx_callback(msg, fdcan->rx_callback_contexts_[filter_index]);
            }
          }
        });
  }

  ~FdCan() {
    HAL_FDCAN_UnRegisterRxFifo0Callback(Handle);
    stm32cubemx_helper::set_context<Handle, FdCan>(nullptr);
  }

  bool start() {
    if (HAL_FDCAN_ConfigGlobalFilter(Handle, FDCAN_REJECT, FDCAN_REJECT,
                                     FDCAN_REJECT_REMOTE,
                                     FDCAN_REJECT_REMOTE) != HAL_OK) {
      return false;
    }
    if (HAL_FDCAN_ActivateNotification(Handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                       0) != HAL_OK) {
      return false;
    }
    return HAL_FDCAN_Start(Handle) == HAL_OK;
  }

  bool stop() {
    if (HAL_FDCAN_Stop(Handle) != HAL_OK) {
      return false;
    }
    return HAL_FDCAN_DeactivateNotification(
               Handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == HAL_OK;
  }

  bool transmit(const CanMessage &msg, uint32_t timeout) {
    FDCAN_TxHeaderTypeDef tx_header = create_tx_header(msg);
    core::TimeoutHelper timeout_helper{timeout};
    while (HAL_FDCAN_AddMessageToTxFifoQ(Handle, &tx_header, msg.data.data()) !=
           HAL_OK) {
      if (timeout_helper.is_timeout()) {
        return false;
      }
      core::yield();
    }
    return true;
  }

  std::optional<size_t> attach_rx_filter(const CanFilter &filter,
                                         void (*callback)(const CanMessage &msg,
                                                          void *context),
                                         void *context) {
    auto filter_index = find_rx_filter_index(filter);
    if (!filter_index) {
      return std::nullopt;
    }
    if (!enable_rx_filter(filter, *filter_index)) {
      return std::nullopt;
    }
    rx_callbacks_[*filter_index] = callback;
    rx_callback_contexts_[*filter_index] = context;
    return filter_index;
  }

  bool detach_rx_filter(size_t filter_index) {
    if (!disable_rx_filter(filter_index)) {
      return false;
    }
    rx_callbacks_[filter_index] = nullptr;
    rx_callback_contexts_[filter_index] = nullptr;
    return true;
  }

private:
  std::vector<void (*)(const CanMessage &msg, void *context)> rx_callbacks_{};
  std::vector<void *> rx_callback_contexts_{};

  FdCan(const FdCan &) = delete;
  FdCan &operator=(const FdCan &) = delete;

  std::optional<size_t> find_rx_filter_index(const CanFilter &filter) {
    if (filter.ide) {
      auto it = std::find(rx_callbacks_.begin() + Handle->Init.StdFiltersNbr,
                          rx_callbacks_.end(), nullptr);
      if (it == rx_callbacks_.end()) {
        return std::nullopt;
      }
      return std::distance(rx_callbacks_.begin(), it);
    } else {
      auto it = std::find(rx_callbacks_.begin(),
                          rx_callbacks_.begin() + Handle->Init.StdFiltersNbr,
                          nullptr);
      if (it == rx_callbacks_.begin() + Handle->Init.StdFiltersNbr) {
        return std::nullopt;
      }
      return std::distance(rx_callbacks_.begin(), it);
    }
  }

  static inline bool enable_rx_filter(const CanFilter &filter,
                                      uint32_t filter_index) {
    FDCAN_FilterTypeDef filter_config{};
    if (filter.ide) {
      filter_config.IdType = FDCAN_EXTENDED_ID;
      filter_config.FilterIndex = filter_index - Handle->Init.StdFiltersNbr;
    } else {
      filter_config.IdType = FDCAN_STANDARD_ID;
      filter_config.FilterIndex = filter_index;
    }
    filter_config.FilterType = FDCAN_FILTER_MASK;
    filter_config.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter_config.FilterID1 = filter.id;
    filter_config.FilterID2 = filter.mask;

    return HAL_FDCAN_ConfigFilter(Handle, &filter_config) == HAL_OK;
  }

  static inline bool disable_rx_filter(size_t filter_index) {
    FDCAN_FilterTypeDef filter_config{};
    if (filter_index >= Handle->Init.StdFiltersNbr) {
      filter_config.IdType = FDCAN_EXTENDED_ID;
      filter_config.FilterIndex = filter_index - Handle->Init.StdFiltersNbr;
    } else {
      filter_config.IdType = FDCAN_STANDARD_ID;
      filter_config.FilterIndex = filter_index;
    }
    filter_config.FilterConfig = FDCAN_FILTER_DISABLE;

    return HAL_FDCAN_ConfigFilter(Handle, &filter_config) == HAL_OK;
  }

  static inline FDCAN_TxHeaderTypeDef create_tx_header(const CanMessage &msg) {
    FDCAN_TxHeaderTypeDef tx_header{};
    tx_header.Identifier = msg.id;
    if (msg.ide) {
      tx_header.IdType = FDCAN_EXTENDED_ID;
    } else {
      tx_header.IdType = FDCAN_STANDARD_ID;
    }
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    switch (msg.dlc) {
    case 0:
      tx_header.DataLength = FDCAN_DLC_BYTES_0;
      break;
    case 1:
      tx_header.DataLength = FDCAN_DLC_BYTES_1;
      break;
    case 2:
      tx_header.DataLength = FDCAN_DLC_BYTES_2;
      break;
    case 3:
      tx_header.DataLength = FDCAN_DLC_BYTES_3;
      break;
    case 4:
      tx_header.DataLength = FDCAN_DLC_BYTES_4;
      break;
    case 5:
      tx_header.DataLength = FDCAN_DLC_BYTES_5;
      break;
    case 6:
      tx_header.DataLength = FDCAN_DLC_BYTES_6;
      break;
    case 7:
      tx_header.DataLength = FDCAN_DLC_BYTES_7;
      break;
    case 8:
      tx_header.DataLength = FDCAN_DLC_BYTES_8;
      break;
    }
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;
    return tx_header;
  }

  static inline void update_rx_message(CanMessage &msg,
                                       const FDCAN_RxHeaderTypeDef &rx_header) {
    msg.id = rx_header.Identifier;
    if (rx_header.IdType == FDCAN_STANDARD_ID) {
      msg.ide = false;
    } else if (rx_header.IdType == FDCAN_EXTENDED_ID) {
      msg.ide = true;
    }
    switch (rx_header.DataLength) {
    case FDCAN_DLC_BYTES_0:
      msg.dlc = 0;
      break;
    case FDCAN_DLC_BYTES_1:
      msg.dlc = 1;
      break;
    case FDCAN_DLC_BYTES_2:
      msg.dlc = 2;
      break;
    case FDCAN_DLC_BYTES_3:
      msg.dlc = 3;
      break;
    case FDCAN_DLC_BYTES_4:
      msg.dlc = 4;
      break;
    case FDCAN_DLC_BYTES_5:
      msg.dlc = 5;
      break;
    case FDCAN_DLC_BYTES_6:
      msg.dlc = 6;
      break;
    case FDCAN_DLC_BYTES_7:
      msg.dlc = 7;
      break;
    case FDCAN_DLC_BYTES_8:
      msg.dlc = 8;
      break;
    }
  }
};

} // namespace halx::peripheral
