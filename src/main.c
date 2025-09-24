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
  struct sigaction sa;

  (void) argc;
  (void) argv;
  
  sa.sa_handler = sigint_hnd;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  ws_br_agent_utils_print_app_banner();

  assert(ws_br_agent_soc_host_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_srv_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_dbus_init() == WS_BR_AGENT_RET_OK);
  
  while(!main_thread_stop) {
    usleep(1000000UL); // 1 second  
  }

  return EXIT_SUCCESS;
}

static void sigint_hnd(int signum)
{
  (void)signum;
  ws_br_agent_srv_deinit();
  ws_br_agent_dbus_deinit();
  ws_br_agent_log_warn("Stop application...\n");
  main_thread_stop = 1;
}