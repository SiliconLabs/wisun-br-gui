/***************************************************************************//**
 * @file ws_br_agent_defs.h
 * @brief Common definitions for Wi-SUN SoC Border Router Agent
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

#ifndef WS_BR_AGENT_DEFS_H
#define WS_BR_AGENT_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Application name
#define WS_BR_AGENT_APP_NAME "Wi-SUN SoC Border Router Brige Agent Service"

/// Version of the Wi-SUN SoC Border Router Agent
#ifndef WS_BR_AGENT_VERSION
#define WS_BR_AGENT_VERSION "1.0.0"
#endif

/// Copyright string
#define WS_BR_AGENT_APP_COPYRIGHT_STR "Copyright (c) 2025 Silicon Labs, Inc."

/// Return codes
#define WS_BR_AGENT_RET_OK 0L

/// Generic error code
#define WS_BR_AGENT_RET_ERR -1L

/// @brief Type for return codes
typedef int32_t ws_br_agent_ret_t;

/// Port number for the Wi-SUN SoC Border Router Agent service
#define WS_BR_AGENT_SERVICE_PORT 11500U

/// Port number of the Wi-SUN SoC Border Router Agent SoC
#define WS_BR_AGENT_SOC_PORT 11501U

/// Maximum size of the Wi-SUN network name
#define WS_BR_AGENT_NETWORK_NAME_SIZE 32

/// Size of the request/response buffer
#define WS_BR_AGENT_MAX_BUF_SIZE 2048U

/// Maximum size of the IPv6 address string
#define WS_BR_AGENT_IPV6_ADDR_STR_SIZE 40U

/// Maximum size of settings strings
#define WS_BR_AGENT_SETTINGS_STR_DATA_MAX_LENGTH 64U

/// IPv6 prefix string size
#define WS_BR_AGENT_IPV6_PREFIX_SIZE  43U

/// Max PHY mode ID count
#define WS_BR_AGENT_MAX_PHY_MODE_ID_COUNT 15U

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_DEFS_H