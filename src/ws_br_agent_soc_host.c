/***************************************************************************//**
 * @file ws_br_agent_soc_host.c
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_utils.h"

static ws_br_agent_ret_t copy_topology(ws_br_agent_soc_host_topology_t * const dst_topology,
                                       const ws_br_agent_soc_host_topology_t * const src_topology);

static const ws_br_agent_settings_t default_host_settings = {
  .network_name = "Wi-SUN Network",
  .network_size = WS_BR_AGENT_NETWORK_SIZE_SMALL,
  .tx_power_ddbm = 200,
  .uc_dwell_interval_ms = 255U,
  .bc_dwell_interval_ms = 255U,
  .max_neighbor_count = 32U,
  .allowed_channels = "0-255",
  .ipv6_prefix = "fd12:3456::/64",
  .max_child_count = 22U,
  .max_security_neighbor_count = 300U,
  .keychain = WS_BR_AGENT_KEYCHAIN_AUTOMATIC,
  .keychain_index = 0U,
  .socket_rx_buffer_size = 2048U,
  .phy.type = WS_BR_AGENT_PHY_CONFIG_FAN11,
  .phy.config.fan11.reg_domain = WS_BR_AGENT_REG_DOMAIN_EU,
  .phy.config.fan11.chan_plan_id = 32U,
  .phy.config.fan11.phy_mode_id = 1U,
};

static ws_br_agent_soc_host_t host = { 0U };

static pthread_mutex_t host_mutex = PTHREAD_MUTEX_INITIALIZER;

static ws_br_agent_soc_host_topology_t host_topology = { 
  .entry_count = 0U, 
  .entries = NULL 
};

ws_br_agent_ret_t ws_br_agent_soc_host_init(void) 
{
  if (pthread_mutex_init(&host_mutex, NULL) != 0) {
    ws_br_agent_log_error("Mutex init failed\n");
    return WS_BR_AGENT_RET_ERR;
  }
  // Set local host with default settings for init
  ws_br_agent_soc_host_set("::1", NULL);
  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_send_req(const ws_br_agent_msg_t * const req_msg, 
                                                ws_br_agent_soc_host_process_resp_cb_t resp_cb)
{
  int sockfd = -1;
  ssize_t r = 0;
  size_t buf_size = 0;
  uint8_t *rxtx_buf = NULL;
  ws_br_agent_msg_t *msg = NULL;

  pthread_mutex_lock(&host_mutex);
  ws_br_agent_log_info("Send '%s' request (0x%08x)...\n", 
                       ws_br_agent_utils_val_to_str(req_msg->msg_code, 
                                                    ws_br_agent_msg_code_strs, 
                                                    "Unknown"), 
                       req_msg->msg_code);

  sockfd = socket(AF_INET6, SOCK_STREAM, 0);

  if (sockfd < 0) {
    ws_br_agent_log_error("Failed: Socket creation\n");
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  if (connect(sockfd, (struct sockaddr *)&host.remote_addr, sizeof(host.remote_addr)) < 0) {
    ws_br_agent_log_error("Failed: Connection to %s:%u\n", host.remote_addr_str, WS_BR_AGENT_SOC_PORT);
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  rxtx_buf = ws_br_agent_msg_build_buf(req_msg, &buf_size);
  if (rxtx_buf == NULL || buf_size < WS_BR_AGENT_MSG_MIN_BUF_SIZE) {
    ws_br_agent_log_error("Failed: Building request\n");
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  if (send(sockfd, rxtx_buf, buf_size, 0) < 0) { 
    ws_br_agent_log_error("Failed: Sending request\n");
    free(rxtx_buf);
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }
  free(rxtx_buf);

  // Receive data
  rxtx_buf = malloc(WS_BR_AGENT_MAX_BUF_SIZE);
  if (rxtx_buf == NULL) {
    ws_br_agent_log_error("Failed: Memory allocation\n");
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  r = recv(sockfd, rxtx_buf, WS_BR_AGENT_MAX_BUF_SIZE, 0);

  // No response or error
  if (!r) {
    close(sockfd);
    free(rxtx_buf);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_OK;
  
  } else if (r < WS_BR_AGENT_MSG_MIN_BUF_SIZE) {
    ws_br_agent_log_error("Failed: Receiving response\n");
    free(rxtx_buf);
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Received response (%ld bytes)\n", r);
  msg = ws_br_agent_msg_parse_buf(rxtx_buf, (size_t)r);
  free(rxtx_buf);

  if (msg == NULL) {
    ws_br_agent_log_error("Failed: Parsing response\n");
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  if (resp_cb != NULL) {
    if (resp_cb(msg) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_warn("Response process callback failed\n");
      ws_br_agent_msg_free(msg);
      close(sockfd);
      pthread_mutex_unlock(&host_mutex);
      return WS_BR_AGENT_RET_ERR;
    }
  }
  ws_br_agent_msg_free(msg);
  close(sockfd);
  ws_br_agent_log_info("OK\n");
  pthread_mutex_unlock(&host_mutex);
  
  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set(const char *addr,
                                           const ws_br_agent_settings_t *const settings)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  pthread_mutex_lock(&host_mutex);
  if (inet_pton(AF_INET6, addr, &host.remote_addr.sin6_addr) != 1) {
    ws_br_agent_log_error("Invalid IPv6 address: %s\n", addr);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  snprintf(host.remote_addr_str, sizeof(host.remote_addr_str), "%s", addr);
  host.remote_addr_str[WS_BR_AGENT_IPV6_ADDR_STR_SIZE - 1] = '\0';

  if (settings != NULL) {
    memcpy(&host.settings, settings, sizeof(ws_br_agent_settings_t));
  } else {
    memcpy(&host.settings, &default_host_settings, sizeof(ws_br_agent_settings_t));
  }
  pthread_mutex_unlock(&host_mutex);
  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_get(ws_br_agent_soc_host_t * const dst_host)
{
  if (dst_host == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(dst_host, &host, sizeof(ws_br_agent_soc_host_t));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set_remote_addr(const struct sockaddr_in6 * const addr)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(&host.remote_addr, addr, sizeof(struct sockaddr_in6));
  host.remote_addr.sin6_port = htons(WS_BR_AGENT_SOC_PORT);
  inet_ntop(AF_INET6, &addr->sin6_addr, host.remote_addr_str, sizeof(host.remote_addr_str));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}
ws_br_agent_ret_t ws_br_agent_soc_host_get_remote_addr(struct sockaddr_in6 * const addr)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(addr, &host.remote_addr, sizeof(struct sockaddr_in6));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

const ws_br_agent_settings_t *ws_br_agent_soc_host_get_default_settings(void)
{
  const ws_br_agent_settings_t *ret = &default_host_settings;
  return ret;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set_settings(const ws_br_agent_settings_t * const settings)
{
  if (settings == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(&host.settings, settings, sizeof(ws_br_agent_settings_t));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_get_settings(ws_br_agent_settings_t * const settings)
{
  if (settings == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(settings, &host.settings, sizeof(ws_br_agent_settings_t));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

static ws_br_agent_ret_t copy_topology(ws_br_agent_soc_host_topology_t * const dst_topology,
                                       const ws_br_agent_soc_host_topology_t * const src_topology)
{
  size_t storage_size = 0U;

  if (src_topology == NULL
     || dst_topology == NULL
     || src_topology->entries == NULL 
     || !src_topology->entry_count) {
    return WS_BR_AGENT_RET_ERR;
  }

  if (dst_topology->entries != NULL) {
    free(dst_topology->entries);
  }

  dst_topology->entry_count = src_topology->entry_count;

  // Allocate dest topology
  storage_size = src_topology->entry_count * sizeof(ws_br_agent_soc_host_topology_entry_t);
  dst_topology->entries = (ws_br_agent_soc_host_topology_entry_t *) malloc(storage_size);
  if (dst_topology->entries == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  memcpy(dst_topology->entries, src_topology->entries, storage_size);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set_topology(const ws_br_agent_soc_host_topology_t *topology)
{
  ws_br_agent_ret_t ret = WS_BR_AGENT_RET_ERR;

  pthread_mutex_lock(&host_mutex);
  ret = copy_topology(&host_topology, topology);
  pthread_mutex_unlock(&host_mutex);

  return ret;
}

ws_br_agent_ret_t ws_br_agent_soc_host_get_topology(ws_br_agent_soc_host_topology_t * const topology)
{
  ws_br_agent_ret_t ret = WS_BR_AGENT_RET_ERR;

  pthread_mutex_lock(&host_mutex);
  ret = copy_topology(topology, &host_topology);
  pthread_mutex_unlock(&host_mutex);

  return ret;
}

ws_br_agent_ret_t ws_br_agent_soc_host_free_topology(ws_br_agent_soc_host_topology_t *topology)
{
  if (topology == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  if (topology->entries != NULL) {
    free(topology->entries);
    topology->entries = NULL;
  }

  topology->entry_count = 0;

  return WS_BR_AGENT_RET_OK;
}