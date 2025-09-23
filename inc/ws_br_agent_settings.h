#ifndef WS_BR_AGENT_SETTINGS_H
#define WS_BR_AGENT_SETTINGS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ws_br_agent_defs.h"
#ifdef __cplusplus
extern "C" {
#endif

/// @brief Structure for Wi-SUN Border Router settings
typedef struct __attribute__((packed, aligned(1))) ws_br_agent_settings {
  /// @brief Network name (null-terminated string)
  char network_name[WS_BR_AGENT_NETWORK_NAME_SIZE + 1];
  /// @brief Regulatory domain
  uint8_t regulatory_domain;
  /// @brief Network size
  uint8_t network_size;
  /// @brief Channel plan ID
  uint8_t chan_plan_id;
  /// @brief PHY mode ID
  uint8_t phy_mode_id;
  /// @brief TX power in dBm (signed)
  int16_t tx_power_ddbm;
  /// @brief PAN ID
  uint16_t pan_id;
  /// @brief GAKs (4 keys, 16 bytes each)
  uint8_t gaks[4][16];
  /// @brief GTKs (4 keys, 16 bytes each)
  uint8_t gtks[4][16];
} ws_br_agent_settings_t;

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_SETTINGS_H