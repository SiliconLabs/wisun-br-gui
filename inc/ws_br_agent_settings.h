#ifndef WS_BR_AGENT_SETTINGS_H
#define WS_BR_AGENT_SETTINGS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ws_br_agent_defs.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((aligned(1))) ws_br_agent_settings {
  char network_name[WS_BR_AGENT_NETWORK_NAME_SIZE + 1];
  uint8_t regulatory_domain;
  uint8_t network_size;
  uint8_t chan_plan_id;
  uint8_t phy_mode_id;
  int16_t tx_power_ddbm;
  uint16_t pan_id;
  uint8_t gaks[4][16];
  uint8_t gtks[4][16];
} ws_br_agent_settings_t;

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_SETTINGS_H