#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <optional>

#include "halx/core.hpp"

namespace halx::peripheral {

template <CAN_HandleTypeDef *Handle> class BxCan {
public:
  BxCan() {
    stm32cubemx_helper::set_context<Handle, BxCan>(this);
    HAL_CAN_RegisterCallback(
        Handle, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID,
        [](CAN_HandleTypeDef *hcan) {
          CAN_RxHeaderTypeDef rx_header;
          CanMessage msg;

          auto bxcan = stm32cubemx_helper::get_context<Handle, BxCan>();

          while (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header,
                                      msg.data.data()) == HAL_OK) {
            if (rx_header.FilterMatchIndex >= BxCan::FILTER_BANK_SIZE) {
              continue;
            }
            auto rx_callback = bxcan->rx_callbacks_[rx_header.FilterMatchIndex];
            if (rx_callback) {
              update_rx_message(msg, rx_header);
              rx_callback(
                  msg,
                  bxcan->rx_callback_contexts_[rx_header.FilterMatchIndex]);
            }
          }
        });
  }

  ~BxCan() {
    HAL_CAN_UnRegisterCallback(Handle, HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID);
    stm32cubemx_helper::set_context<Handle, BxCan>(nullptr);
  }

  bool start() {
    if (HAL_CAN_ActivateNotification(Handle, CAN_IT_RX_FIFO0_MSG_PENDING) !=
        HAL_OK) {
      return false;
    }
    return HAL_CAN_Start(Handle) == HAL_OK;
  }

  bool stop() {
    if (HAL_CAN_Stop(Handle) != HAL_OK) {
      return false;
    }
    return HAL_CAN_DeactivateNotification(
               Handle, CAN_IT_RX_FIFO0_MSG_PENDING) == HAL_OK;
  }

  bool transmit(const CanMessage &msg, uint32_t timeout) {
    CAN_TxHeaderTypeDef tx_header = create_tx_header(msg);
    uint32_t tx_mailbox;
    core::TimeoutHelper timeout_helper{timeout};
    while (HAL_CAN_AddTxMessage(Handle, &tx_header, msg.data.data(),
                                &tx_mailbox) != HAL_OK) {
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
    return true;
  }

private:
  static constexpr uint32_t FILTER_BANK_SIZE = 14;

  std::array<void (*)(const CanMessage &msg, void *context), FILTER_BANK_SIZE>
      rx_callbacks_{};
  std::array<void *, FILTER_BANK_SIZE> rx_callback_contexts_{};

  BxCan(const BxCan &) = delete;
  BxCan &operator=(const BxCan &) = delete;

  std::optional<size_t> find_rx_filter_index(const CanFilter &) {
    auto it = std::find(rx_callbacks_.begin(), rx_callbacks_.end(), nullptr);
    if (it == rx_callbacks_.end()) {
      return std::nullopt;
    }
    return std::distance(rx_callbacks_.begin(), it);
  }

  static inline bool enable_rx_filter(const CanFilter &filter,
                                      uint32_t filter_index) {
    CAN_FilterTypeDef filter_config{};
    if (filter.ide) {
      filter_config.FilterIdHigh = filter.id >> 13;
      filter_config.FilterIdLow = ((filter.id << 3) & 0xFFFF) | 0x4;
      filter_config.FilterMaskIdHigh = filter.mask >> 13;
      filter_config.FilterMaskIdLow = ((filter.mask << 3) & 0xFFFF) | 0x4;
    } else {
      filter_config.FilterIdHigh = filter.id << 5;
      filter_config.FilterIdLow = 0x0;
      filter_config.FilterMaskIdHigh = filter.mask << 5;
      filter_config.FilterMaskIdLow = 0x0;
    }
    filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter_config.FilterBank = filter_index;
#ifdef CAN2
    if (Handle->Instance == CAN2) {
      filter_config.FilterBank += FILTER_BANK_SIZE;
    }
#endif
    filter_config.FilterMode = CAN_FILTERMODE_IDMASK;
    filter_config.FilterScale = CAN_FILTERSCALE_32BIT;
    filter_config.FilterActivation = ENABLE;
    filter_config.SlaveStartFilterBank = FILTER_BANK_SIZE;

    return HAL_CAN_ConfigFilter(Handle, &filter_config) == HAL_OK;
  }

  static inline bool disable_rx_filter(size_t filter_index) {
    CAN_FilterTypeDef filter_config{};
    filter_config.FilterBank = filter_index;
#ifdef CAN2
    if (Handle->Instance == CAN2) {
      filter_config.FilterBank += FILTER_BANK_SIZE;
    }
#endif
    filter_config.FilterActivation = DISABLE;

    return HAL_CAN_ConfigFilter(Handle, &filter_config) == HAL_OK;
  }

  static inline CAN_TxHeaderTypeDef create_tx_header(const CanMessage &msg) {
    CAN_TxHeaderTypeDef tx_header{};
    if (msg.ide) {
      tx_header.ExtId = msg.id;
      tx_header.IDE = CAN_ID_EXT;
    } else {
      tx_header.StdId = msg.id;
      tx_header.IDE = CAN_ID_STD;
    }
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = msg.dlc;
    tx_header.TransmitGlobalTime = DISABLE;
    return tx_header;
  }

  static inline void update_rx_message(CanMessage &msg,
                                       const CAN_RxHeaderTypeDef &rx_header) {
    switch (rx_header.IDE) {
    case CAN_ID_STD:
      msg.id = rx_header.StdId;
      msg.ide = false;
      break;
    case CAN_ID_EXT:
      msg.id = rx_header.ExtId;
      msg.ide = true;
      break;
    }
    msg.dlc = rx_header.DLC;
  }
};

} // namespace halx::peripheral
