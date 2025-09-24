/***************************************************************************//**
 * @file ws_br_agent_settings.h
 * @brief Soc Border Router settings
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef WS_BR_AGENT_SETTINGS_H
#define WS_BR_AGENT_SETTINGS_H

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