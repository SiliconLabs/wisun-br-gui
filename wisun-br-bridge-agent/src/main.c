/***************************************************************************//**
 * @file main.c
 * @brief Wi-SUN SoC Border Router Agent application
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

#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_srv.h"
#include "ws_br_agent_utils.h"
#include "ws_br_agent_dbus.h"

static void sigint_hnd(int signum);
static volatile sig_atomic_t main_thread_stop = 0;

int main(int argc, char *argv[])
{
  const char *conf_file_path = NULL;
  const char *soc_host_addr = NULL;
  ws_br_agent_msg_t msg = { 0U };
  ws_br_agent_settings_t settings = { 0U };

  struct sockaddr_in6 new_addr = { 
    .sin6_family = AF_INET6, 
    .sin6_port = htons(WS_BR_AGENT_SOC_PORT)
  };
  
  struct sigaction sa;
  int opt;

  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--log")
        || !strcmp(argv[i], "-l") && (i + 1 < argc)) {
      ws_br_agent_log_file_path = argv[i + 1];
      ++i;
    }
    else if (!strcmp(argv[i], "--config")
             || !strcmp(argv[i], "-c") && (i + 1 < argc)) {
      // parse settings
      conf_file_path = argv[i + 1];
      ++i;
    }
    else if (!strcmp(argv[i], "--soc") ||
              !strcmp(argv[i], "-s") && (i + 1 < argc)) {
        // set SoC device path
        soc_host_addr = argv[i + 1];
        ++i;
    }
    else if (!strcmp(argv[i], "--help")
             || !strcmp(argv[i], "-h")) {
      ws_br_agent_utils_print_help();
      exit(EXIT_SUCCESS);
    }
    else if (!strcmp(argv[i], "--version")
             || !strcmp(argv[i], "-v")) {
      ws_br_agent_utils_print_version();
      exit(EXIT_SUCCESS);
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      ws_br_agent_utils_print_help();
      exit(EXIT_FAILURE);
    }
  }

  // Print app banner
  ws_br_agent_utils_print_app_banner();

  assert(ws_br_agent_log_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_soc_host_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_srv_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_dbus_init() == WS_BR_AGENT_RET_OK);
  
  sa.sa_handler = sigint_hnd;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  if (conf_file_path != NULL) {
    ws_br_agent_soc_host_update_settings(conf_file_path);
  }

  if (soc_host_addr != NULL) {
    if (inet_pton(AF_INET6, soc_host_addr, &new_addr.sin6_addr) != 1) {
      ws_br_agent_log_error("Invalid SoC Host IPv6 address: %s\n", soc_host_addr);
      return EXIT_FAILURE;
    }
    if (ws_br_agent_soc_host_set_remote_addr(&new_addr) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_error("Failed to set SoC Host remote address: %s\n", soc_host_addr);
      return EXIT_FAILURE;
    }
    ws_br_agent_log_info("Set SoC Host remote address: %s\n", soc_host_addr);
    (void) ws_br_agent_soc_host_get_settings(&settings);

    msg.msg_code = WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS;
    msg.payload = (uint8_t *)&settings;
    msg.payload_len = sizeof(ws_br_agent_settings_t);
    if (ws_br_agent_soc_host_send_req(&msg, NULL) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_warn("Failed to send SoC Host request\n");
    }
  }

  while (!main_thread_stop) {
    usleep(100000UL);
  }

  return EXIT_SUCCESS;
}

static void sigint_hnd(int signum)
{
  (void) signum;
  ws_br_agent_srv_deinit();
  ws_br_agent_dbus_deinit();
  ws_br_agent_log_warn("Stop application...\n");
  ws_br_agent_log_deinit();
  main_thread_stop = 1;
}