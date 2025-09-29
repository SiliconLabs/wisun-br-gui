/***************************************************************************//**
 * @file ws_br_agent_srv.c
 * @brief Border Router Agent service
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
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_utils.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_dbus.h"
#include "ws_br_agent_srv.h"

#define DISPACH_DELAY_US 1000UL

static pthread_t srv_thr;
static volatile sig_atomic_t srv_thread_stop = 0;
static int listen_fd = -1L;
static void srv_thr_fnc(void *arg);
static ws_br_agent_ret_t handle_topology_req(const ws_br_agent_msg_t *const req_msg);
static ws_br_agent_ret_t handle_set_config_params_req(const ws_br_agent_msg_t *const req_msg,
                                                      const struct sockaddr_in6 * const clnt_addr);
static ws_br_agent_ret_t handle_get_config_params_req(int conn_fd);

ws_br_agent_ret_t ws_br_agent_srv_init(void)
{
  if (pthread_create(&srv_thr, NULL, (void *)srv_thr_fnc, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }
  return WS_BR_AGENT_RET_OK;
}

void ws_br_agent_srv_deinit(void)
{
  srv_thread_stop = 1;
  // Connect to the listening socket to unblock accept()
  struct sockaddr_in6 addr = {0};
  int sock = socket(AF_INET6, SOCK_STREAM, 0);
  if (sock >= 0) {
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_loopback;
    addr.sin6_port = htons(WS_BR_AGENT_SERVICE_PORT);
    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);
  }
  close(listen_fd);
  pthread_join(srv_thr, NULL);
}

static void srv_thr_fnc(void *arg)
{

  int conn_fd = -1L;
  struct sockaddr_in6 serv_addr = {0U};
  struct sockaddr_in6 client_addr = {0U};
  socklen_t client_len = sizeof(client_addr);
  char client_ip[INET6_ADDRSTRLEN] = {0U};
  static uint8_t buf[WS_BR_AGENT_MAX_BUF_SIZE] = {0U};
  ssize_t r = 0L;
  ws_br_agent_msg_t *msg = NULL;
  ws_br_agent_soc_host_topology_t topology = {0U, NULL};

  (void)arg;
  ws_br_agent_log_warn("Server thread started\n");

  listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (listen_fd < 0)
  {
    ws_br_agent_log_error("Server socket creation failed\n");
    return;
  }

  int optval = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
    ws_br_agent_log_warn("Failed to set SO_REUSEADDR\n");
  }

  serv_addr.sin6_family = AF_INET6;
  serv_addr.sin6_addr = in6addr_any;
  serv_addr.sin6_port = htons(WS_BR_AGENT_SERVICE_PORT);

  if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    ws_br_agent_log_error("Server bind failed\n");
    close(listen_fd);
    return;
  }

  if (listen(listen_fd, 5) < 0)
  {
    ws_br_agent_log_error("Server listen failed\n");
    close(listen_fd);
    return;
  }

  ws_br_agent_log_info("Server listening on port %d\n", WS_BR_AGENT_SERVICE_PORT);

  while (!srv_thread_stop) {
    conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (conn_fd < 0) {
      if (srv_thread_stop) {
        break;
      }
      ws_br_agent_log_warn("Accept failed\n");
      usleep(DISPACH_DELAY_US);
      continue;
    }
    inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip, sizeof(client_ip));
    ws_br_agent_log_info("Accepted connection from %s:%d\n", client_ip, ntohs(client_addr.sin6_port));

    r = recv(conn_fd, buf, WS_BR_AGENT_MAX_BUF_SIZE, 0);
    if (r < 0) {
      ws_br_agent_log_warn("Receive failed\n");
      close(conn_fd);
      continue;
    }
    else if (r == 0) {
      ws_br_agent_log_warn("Connection closed by client\n");
      close(conn_fd);
      continue;
    }

    msg = ws_br_agent_msg_parse_buf(buf, (size_t)r);
    if (msg == NULL) {
      ws_br_agent_log_warn("Failed to parse received message\n");
      close(conn_fd);
      continue;
    }

    // Print message
    ws_br_agent_utils_print_msg(msg);

    // Handle requests
    switch (msg->msg_code) {
    // Handle topology request
    case WS_BR_AGENT_MSG_CODE_TOPOLOGY:
      if (handle_topology_req(msg) != WS_BR_AGENT_RET_OK) {
        break;
      }
      if (ws_br_agent_dbus_notify_topology_changed() != WS_BR_AGENT_RET_OK) {
        ws_br_agent_log_error("Failed to notify topology changed via D-Bus\n");
      }
      break;

    // Handle set config request: Used for subscription
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      if (handle_set_config_params_req(msg, &client_addr) != WS_BR_AGENT_RET_OK) {
        break;
      }
      if (ws_br_agent_dbus_notify_settings_changed() != WS_BR_AGENT_RET_OK) {
        ws_br_agent_log_error("Failed to notify settings changed via D-Bus\n");
      }
      break;

    // Not handled requests
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
      (void) handle_get_config_params_req(conn_fd);
      break;
    
    case WS_BR_AGENT_MSG_CODE_START_BR:
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
      ws_br_agent_log_warn("Not handled request: '%s' (0x%08x)\n",
                           ws_br_agent_utils_get_req_code_str(msg->msg_code), msg->msg_code);
      break;
    default:
      ws_br_agent_log_warn("Unknown request: (0x%08x)\n", msg->msg_code);
      break;
    }

    // Free message
    ws_br_agent_msg_free(msg);

    close(conn_fd);
  }
  close(listen_fd);
  ws_br_agent_log_warn("Server thread stopped\n");
}

static ws_br_agent_ret_t handle_topology_req(const ws_br_agent_msg_t *const req_msg)
{
  ws_br_agent_soc_host_topology_t topology = {0U, NULL};

  if (req_msg == NULL || 
      req_msg->payload_len % sizeof(ws_br_agent_soc_host_topology_entry_t)) {
    ws_br_agent_log_error("Failed to handle TOPOLOGY request\n");
    return WS_BR_AGENT_RET_ERR;
  }

  topology.entry_count = req_msg->payload_len / sizeof(ws_br_agent_soc_host_topology_entry_t);
  topology.entries = (ws_br_agent_soc_host_topology_entry_t *)req_msg->payload;
  ws_br_agent_log_info("Topology updated, total %u entries\n", topology.entry_count);
  return ws_br_agent_soc_host_set_topology(&topology);
}

static ws_br_agent_ret_t handle_set_config_params_req(const ws_br_agent_msg_t *const req_msg,
                                                      const struct sockaddr_in6 * const clnt_addr)
{
  ws_br_agent_settings_t settings = { 0U };

  if (clnt_addr == NULL || req_msg == NULL || req_msg->payload == NULL 
      || req_msg->payload_len != sizeof(ws_br_agent_settings_t)) {
    ws_br_agent_log_error("Bad SET_CONFIG_PARAMS request\n");
    return WS_BR_AGENT_RET_ERR;
  }

  
  if (ws_br_agent_soc_host_set_remote_addr(clnt_addr) != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("Failed to set remote address\n");
    return WS_BR_AGENT_RET_ERR;
  }
  
  if (ws_br_agent_soc_host_set_settings((ws_br_agent_settings_t *)req_msg->payload) 
      != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("Failed to set host settings\n");
    return WS_BR_AGENT_RET_ERR;
  }
  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("Failed to check host settings\n");
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Config params updated:\n");

  ws_br_agent_utils_print_host_settings(&settings);

  return WS_BR_AGENT_RET_OK;
}

static ws_br_agent_ret_t handle_get_config_params_req(int conn_fd)
{
  uint8_t *buf = NULL;
  size_t buf_size = 0U;
  ws_br_agent_msg_t msg = { 
    .msg_code = WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS,
    .payload_len = 0U,
    .payload = NULL
  };

  buf = ws_br_agent_msg_build_buf(&msg, &buf_size);

  if (buf == NULL || buf_size != WS_BR_AGENT_MSG_SET_PARAM_MSG_BUF_SIZE) {
    ws_br_agent_log_error("Failed to build SET_CONFIG_PARAMS as response\n");
    return WS_BR_AGENT_RET_ERR;
  }

  if (send(conn_fd, buf, buf_size, 0) < 0) {
    ws_br_agent_log_error("Failed to send SET_CONFIG_PARAMS as response\n");
    free(buf);
    return WS_BR_AGENT_RET_ERR;
  }

  free(buf);

  return WS_BR_AGENT_RET_OK;
}
