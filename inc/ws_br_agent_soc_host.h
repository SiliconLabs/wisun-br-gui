/***************************************************************************//**
 * @file ws_br_agent_soc_host.h
 * @brief Wi-SUN SoC Border Router Agent client interface
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

#ifndef WS_BR_AGENT_CLNT_H
#define WS_BR_AGENT_CLNT_H

#include <netinet/in.h>

#include "ws_br_agent_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Wi-SUN SoC Border Router host information
typedef struct ws_br_agent_soc_host {
  /// @brief Local IPv6 address string
  char remote_addr_str[WS_BR_AGENT_IPV6_ADDR_STR_SIZE];
  /// @brief Remote IPv6 address
  struct sockaddr_in6 remote_addr;
  /// @brief Border Router settings
  ws_br_agent_settings_t settings;
} ws_br_agent_soc_host_t;

/// @brief Topology entry
typedef struct __attribute__((packed, aligned(1))) 
ws_br_agent_soc_host_topology_entry {
  /// @brief GUA/ULA of the routed node
  uint8_t target[16];
  /// @brief GUA/ULA of the routed node's preferred parent
  uint8_t preferred[16];
  /// @brief GUA/ULA of the routed node's backup parent. INADDR_ANY if not set.
  uint8_t backup[16];
} ws_br_agent_soc_host_topology_entry_t;

/// @brief Topology information
typedef struct ws_br_agent_soc_host_topology {
  /// @brief Number of entries
  uint8_t entry_count;
  /// @brief Pointer to the entries (dynamically allocated, NULL if entry_count is 0)
  ws_br_agent_soc_host_topology_entry_t *entries;
} ws_br_agent_soc_host_topology_t;

/// @brief Callback type for processing responses from the SoC
typedef ws_br_agent_ret_t (*ws_br_agent_soc_host_process_resp_cb_t)
                           (const ws_br_agent_msg_t * const msg);

/**
 * @brief Initialize the Wi-SUN SoC Border Router Agent client module.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_init(void);

/**
 * @brief Send a request message to the SoC and optionally process the response.
 * @details This function establishes a TCP connection to the SoC, sends the request message,
 * @param[in] req_msg Pointer to the request message structure.
 * @param[in] resp_cb Optional callback function to process the response message. Can be NULL if no response processing is needed.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_send_req(const ws_br_agent_msg_t * const req_msg, 
                                                ws_br_agent_soc_host_process_resp_cb_t resp_cb);

/**
 * @brief Set the SoC host address and optionally the settings.
 * @details If settings is NULL, default settings will be used.
 * @param[in] addr IPv6 address string of the SoC host.
 * @param[in] settings Optional pointer to the settings structure. If NULL, default settings will be used.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_set(const char *addr,
                                           const ws_br_agent_settings_t *const settings);


/**
 * @brief Get the current SoC host information.
 * @details Copies the current host information into the provided structure.
 * @param[out] dst_host Pointer to the destination host structure to fill.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_get(ws_br_agent_soc_host_t * const dst_host);

/**
 * @brief Set the remote IPv6 address of the SoC host.
 * @param[in] addr Pointer to the IPv6 address structure to set.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_set_remote_addr(const struct sockaddr_in6 * const addr);

/**
 * @brief Get the remote IPv6 address of the SoC host.
 * @param[out] addr Pointer to the IPv6 address structure to fill.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_get_remote_addr(struct sockaddr_in6 * const addr);

/**
 * @brief Get a pointer to the default settings structure.
 * @return Pointer to the default settings structure.
 */
const ws_br_agent_settings_t *ws_br_agent_soc_host_get_default_settings(void);

/**
 * @brief Set the current settings for the SoC host.
 * @param[in] settings Pointer to the settings structure to set.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_set_settings(const ws_br_agent_settings_t * const settings);

/**
 * @brief Get the current settings for the SoC host.
 * @param[out] settings Pointer to the settings structure to fill.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_get_settings(ws_br_agent_settings_t * const settings);

/**
 * @brief Set the current topology information for the SoC host.
 * @param[in] topology Pointer to the topology structure to set.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_set_topology(const ws_br_agent_soc_host_topology_t *topology);

/**
 * @brief Get the current topology information for the SoC host.
 * @param[out] topology Pointer to the topology structure to fill.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_get_topology(ws_br_agent_soc_host_topology_t * const topology);

/**
 * @brief Free memory allocated for topology entries.
 * @param[in,out] topology Pointer to the topology structure whose entries will be freed.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_soc_host_free_topology(ws_br_agent_soc_host_topology_t * const topology);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_CLNT_H