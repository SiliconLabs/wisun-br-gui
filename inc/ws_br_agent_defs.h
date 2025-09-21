#ifndef WS_BR_AGENT_DEFS_H
#define WS_BR_AGENT_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Application name
#define WS_BR_AGENT_APP_NAME "Wi-SUN SoC Border Router Agent Service"

/// Version of the Wi-SUN SoC Border Router Agent
#ifndef WS_BR_AGENT_VERSION
#define WS_BR_AGENT_VERSION "0.1"
#endif

/// Copyright string
#define WS_BR_AGENT_APP_COPYRIGHT_STR "Copyright (c) 2025 Silicon Labs, Inc."

/// Return codes
#define WS_BR_AGENT_RET_OK 0L
#define WS_BR_AGENT_RET_ERR -1L

typedef int32_t ws_br_agent_ret_t;

/// Port number for the Wi-SUN SoC Border Router Agent service
#define WS_BR_AGENT_SERVICE_PORT 5678U

/// Port number of the Wi-SUN SoC Border Router Agent SoC
#define WS_BR_AGENT_SOC_PORT 4567U

/// Maximum size of the Wi-SUN network name
#define WS_BR_AGENT_NETWORK_NAME_SIZE 32

/// Size of the request/response buffer
#define WS_BR_AGENT_MAX_BUF_SIZE 2048U

/// Maximum size of the IPv6 address string
#define WS_BR_AGENT_IPV6_ADDR_STR_SIZE 40U

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_DEFS_H