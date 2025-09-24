/***************************************************************************//**
 * @file ws_br_agent_log.c
 * @brief Logging implementation for Wi-SUN SoC Border Router Agent
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

#include "ws_br_agent_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

const char *ws_br_agent_log_file_path = WS_BR_AGENT_LOG_DEFAULT_FILE_PATH;
FILE *_log_file = NULL;
pthread_mutex_t _log_mutex = PTHREAD_MUTEX_INITIALIZER;

ws_br_agent_ret_t ws_br_agent_log_init(void) 
{
#if WS_BR_AGENT_LOG_ENABLE_FILE_LOG

  _log_file = fopen(ws_br_agent_log_file_path, "a");
  if (_log_file == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  if (pthread_mutex_init(&_log_mutex, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }
  ws_br_agent_log_info("Log file: %s\n", ws_br_agent_log_file_path);
#else
  (void) _log_file;
#endif
  return WS_BR_AGENT_RET_OK;
}

void ws_br_agent_log_deinit(void) 
{
#if WS_BR_AGENT_LOG_ENABLE_FILE_LOG
  pthread_mutex_lock(&_log_mutex);
  fclose(_log_file);
  _log_file = NULL;
  pthread_mutex_unlock(&_log_mutex);
#else
  (void) _log_file;
#endif
}

const char *_log_get_timestr(void) {
  static char buf[24];
  time_t now = time(NULL);
  struct tm tm_now;
  
  localtime_r(&now, &tm_now);
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_now);
  
  return buf;
}
