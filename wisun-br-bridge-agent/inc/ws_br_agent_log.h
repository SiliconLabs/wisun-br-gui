/***************************************************************************//**
 * @file ws_br_agent_log.h
 * @brief Logging macros for Wi-SUN SoC Border Router Agent
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

#ifndef WS_BR_AGENT_LOG_H
#define WS_BR_AGENT_LOG_H

#include <stdio.h>
#include <pthread.h>

#include "ws_br_agent_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Enable/Disable colored log output
#ifndef WS_BR_AGENT_LOG_ENABLE_COLORS
#define WS_BR_AGENT_LOG_ENABLE_COLORS       1U
#endif

#ifndef WS_BR_AGENT_LOG_ENABLE_DEBUG
#define WS_BR_AGENT_LOG_ENABLE_DEBUG        1U
#endif

#ifndef WS_BR_AGENT_LOG_ENABLE_CONSOLE_LOG
#define WS_BR_AGENT_LOG_ENABLE_CONSOLE_LOG  1U
#endif

#ifndef WS_BR_AGENT_LOG_ENABLE_FILE_LOG
#define WS_BR_AGENT_LOG_ENABLE_FILE_LOG     1U
#endif

/// Color definitions for terminal output
#if WS_BR_AGENT_LOG_ENABLE_COLORS
#define WS_BR_AGENT_LOG_COLOR_RED     "\x1b[31m"
#define WS_BR_AGENT_LOG_COLOR_GREEN   "\x1b[32m"
#define WS_BR_AGENT_LOG_COLOR_YELLOW  "\x1b[33m"
#define WS_BR_AGENT_LOG_COLOR_BLUE    "\x1b[34m"
#define WS_BR_AGENT_LOG_COLOR_MAGENTA "\x1b[35m"
#define WS_BR_AGENT_LOG_COLOR_CYAN    "\x1b[36m"
#define WS_BR_AGENT_LOG_COLOR_WHITE   "\x1b[37m"
#define WS_BR_AGENT_LOG_COLOR_RESET   "\x1b[0m"
#else
#define WS_BR_AGENT_LOG_COLOR_RED     ""
#define WS_BR_AGENT_LOG_COLOR_GREEN   ""
#define WS_BR_AGENT_LOG_COLOR_YELLOW  ""
#define WS_BR_AGENT_LOG_COLOR_BLUE    ""
#define WS_BR_AGENT_LOG_COLOR_MAGENTA ""
#define WS_BR_AGENT_LOG_COLOR_CYAN    ""
#define WS_BR_AGENT_LOG_COLOR_WHITE   ""
#define WS_BR_AGENT_LOG_COLOR_RESET   ""
#endif

#define WS_BR_AGENT_LOG_DEFAULT_FILE_PATH "/var/log/wisun-soc-br-agent.log"

/// Log file path (default: /var/log/wisun-soc-br-agent.log)
extern const char *ws_br_agent_log_file_path;
extern FILE *_log_file;
extern pthread_mutex_t _log_mutex;

/**
 * @brief Init logging
 * @brief Open the log file for appending (default: /var/log/wisun-soc-br-agent.log) 
 *        and init logging mutex
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_log_init(void);

/**
 * @brief Deinit logging
 * @brief Close the log file
 */
void ws_br_agent_log_deinit(void);

/**
 * @brief Get current time string (Internal use only)
 * @return Pointer to a static string containing the current time in "YYYY-MM-DD HH:MM:SS" format.
 */
const char *_log_get_timestr(void);

/// Logging macros
/// @brief Print application banner
#define ws_br_agent_app_print(fmt, ...)                        \
  do {                                                         \
    fprintf(stdout, WS_BR_AGENT_LOG_COLOR_BLUE fmt             \
            WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__);       \
    fflush(stdout);                                            \
  } while (0)

/// @brief Log printer to file (internal use only)
#if WS_BR_AGENT_LOG_ENABLE_FILE_LOG
#define __log_print_to_file(level, fmt, ...)                   \
  do {                                                         \
    if (_log_file) {                                           \
      fprintf(_log_file, "%s ", _log_get_timestr());           \
      fprintf(_log_file, "[" level "] " fmt, ##__VA_ARGS__);   \
      fflush(_log_file);                                       \
    }                                                          \
  } while (0)
#else
#define __log_print_to_file(level, fmt, ...)                   \
  do {                                                         \
    (void)level; (void)fmt; (void)#__VA_ARGS__;                \
  } while (0)
#endif

/// @brief Log printer to console (internal use only)
#if WS_BR_AGENT_LOG_ENABLE_CONSOLE_LOG
#define __log_print_to_console(color, level, fmt, ...)         \
  do {                                                         \
    fprintf(stdout, color "[" level "] " fmt                   \
            WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__);       \
    fflush(stdout);                                            \
  } while (0)
#else 
#define __log_print_to_console(color, level, fmt, ...)         \
  do {                                                         \
    (void)color; (void)level; (void)fmt; (void)#__VA_ARGS__;   \
  } while (0)

#endif

/// @brief Info log printer
#define ws_br_agent_log_info(fmt, ...)                         \
do {                                                           \
  pthread_mutex_lock(&_log_mutex);                             \
  __log_print_to_console(WS_BR_AGENT_LOG_COLOR_WHITE, "INFO",  \
                         fmt, ##__VA_ARGS__);                  \
  __log_print_to_file("INFO", fmt, ##__VA_ARGS__);             \
  pthread_mutex_unlock(&_log_mutex);                           \
} while (0)

/// @brief Warning log printer
#define ws_br_agent_log_warn(fmt, ...)                         \
do {                                                           \
  pthread_mutex_lock(&_log_mutex);                             \
  __log_print_to_console(WS_BR_AGENT_LOG_COLOR_YELLOW, "WARN", \
                         fmt, ##__VA_ARGS__);                  \
  __log_print_to_file("WARN", fmt, ##__VA_ARGS__);             \
  pthread_mutex_unlock(&_log_mutex);                           \
} while (0)

/// @brief Error log printer
#define ws_br_agent_log_error(fmt, ...)                        \
do {                                                           \
  pthread_mutex_lock(&_log_mutex);                             \
  __log_print_to_console(WS_BR_AGENT_LOG_COLOR_RED, "ERROR",   \
                         fmt, ##__VA_ARGS__);                  \
  __log_print_to_file("ERROR", fmt, ##__VA_ARGS__);            \
  pthread_mutex_unlock(&_log_mutex);                           \
} while (0)

/// @brief Debug log printer
#if WS_BR_AGENT_LOG_ENABLE_DEBUG
#define ws_br_agent_log_debug(fmt, ...)                        \
do {                                                           \
  pthread_mutex_lock(&_log_mutex);                             \
  __log_print_to_console(WS_BR_AGENT_LOG_COLOR_CYAN, "DEBUG",  \
                         fmt, ##__VA_ARGS__);                  \
  __log_print_to_file("DEBUG", fmt, ##__VA_ARGS__);            \
  pthread_mutex_unlock(&_log_mutex);                           \
} while (0)
#else
#define ws_br_agent_log_debug(fmt, ...)                        \
  do {                                                         \
    (void)fmt;                                                 \
    (void)#__VA_ARGS__;                                        \
  } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_LOG_H