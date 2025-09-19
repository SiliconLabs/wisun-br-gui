#ifndef WS_BR_AGENT_MSG_H
#define WS_BR_AGENT_MSG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ws_br_agent_defs.h"

#ifdef __cplusplus
extern "C" {
#endif


// Agent Service Request/Response codes
#define WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY       (0x00000001U)
#define WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS  (0x00000002U)
#define WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS  (0x00000003U)
#define WS_BR_AGENT_MSG_CODE_START_BR           (0x00000004U)
#define WS_BR_AGENT_MSG_CODE_STOP_BR            (0x00000005U)

#define WS_BR_AGENT_MSG_DEFAULT_BUF_SIZE (2 * sizeof(uint32_t))
#define WS_BR_AGENT_MSG_SET_PARAM_MSG_BUF_SIZE (WS_BR_AGENT_MSG_DEFAULT_BUF_SIZE + sizeof(ws_br_agent_param_payload_t))

typedef struct __attribute__((aligned(1))) ws_br_agent_param_payload {
  char network_name[WS_BR_AGENT_NETWORK_NAME_SIZE + 1];
  uint8_t regulatory_domain;
  uint8_t network_size;
  uint8_t chan_plan_id;
  uint8_t phy_mode_id;
  int16_t tx_power_ddbm;
  uint16_t pan_id;
  uint8_t gaks[4][16];
  uint8_t gtks[4][16];
} ws_br_agent_param_payload_t;

typedef uint32_t ws_br_agent_msg_code_t;
typedef uint32_t ws_br_agent_msg_len_t;

/// Packet structure:
/// [msg code 4 byte] [payload len 4 byte] [payload data n byte]
typedef struct ws_br_agent_msg {
  ws_br_agent_msg_code_t msg_code;
  ws_br_agent_msg_len_t payload_len;
  uint8_t *payload;
} ws_br_agent_msg_t;

int32_t ws_br_msg_build_buf(ws_br_agent_msg_t * const msg, size_t *buf_size);

ws_br_agent_msg_t *ws_br_msg_parse_buf(const uint8_t * const buf, const size_t buf_size);

void ws_br_msg_free(ws_br_agent_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_MSG_H