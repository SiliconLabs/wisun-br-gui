#ifndef WS_BR_AGENT_LOG_H
#define WS_BR_AGENT_LOG_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Enable/Disable colored log output
#ifndef WS_BR_AGENT_LOG_ENABLE_COLORS
#define WS_BR_AGENT_LOG_ENABLE_COLORS 1
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

/// Logging macros
/// Print application banner
#define ws_br_agent_app_print(fmt, ...)                                                           \
  do {                                                                                            \
      fprintf(stdout, WS_BR_AGENT_LOG_COLOR_BLUE fmt WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__); \
      fflush(stdout);                                                                             \
  } while (0)

/// Info, Warning, Error, and Debug logs
#define ws_br_agent_log_info(fmt, ...)                                                                            \
  do {                                                                                                            \
      fprintf(stdout, WS_BR_AGENT_LOG_COLOR_WHITE "[INFO] " fmt WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__); \
      fflush(stdout);                                                                                             \
  } while (0)

#define ws_br_agent_log_warn(fmt, ...)                                                                             \
  do {                                                                                                             \
      fprintf(stdout, WS_BR_AGENT_LOG_COLOR_YELLOW "[WARN] " fmt WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__); \
      fflush(stdout);                                                                                              \
  } while (0)

#define ws_br_agent_log_error(fmt, ...)                                                                          \
  do {                                                                                                           \
      fprintf(stderr, WS_BR_AGENT_LOG_COLOR_RED "[ERROR] " fmt WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__); \
      fflush(stderr);                                                                                            \
  } while (0)

#define ws_br_agent_log_debug(fmt, ...)                                                                           \
  do {                                                                                                            \
      fprintf(stdout, WS_BR_AGENT_LOG_COLOR_CYAN "[DEBUG] " fmt WS_BR_AGENT_LOG_COLOR_RESET, ##__VA_ARGS__); \
      fflush(stdout);                                                                                             \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_LOG_H