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
#define WS_BR_AGENT_RET_OK 0
#define WS_BR_AGENT_RET_ERR -1

/// Port number for the Wi-SUN SoC Border Router Agent service
#define WS_BR_AGENT_SERVICE_PORT 5678U

/// Port number of the Wi-SUN SoC Border Router Agent SoC
#define WS_BR_AGENT_SOC_PORT 4567U


// Agent Service Request/Response codes
#define WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY       (0x01U)
#define WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS  (0x02U)
#define WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS  (0x03U)
#define WS_BR_AGENT_MSG_CODE_START_BR           (0x04U)
#define WS_BR_AGENT_MSG_CODE_STOP_BR            (0x05U)


/// Maximum size of the Wi-SUN network name
#define SL_WISUN_NETWORK_NAME_SIZE 32


typedef struct __attribute__((aligned(1))) ws_br_agent_param_payload {
  char network_name[SL_WISUN_NETWORK_NAME_SIZE + 1];
  uint8_t regulatory_domain;
  uint8_t network_size;
  uint8_t chan_plan_id;
  uint8_t phy_mode_id;
  int16_t tx_power_ddbm;
  uint16_t pan_id;
  uint8_t gaks[4][16];
  uint8_t gtks[4][16];
} ws_br_agent_param_payload_t;


#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_DEFS_H