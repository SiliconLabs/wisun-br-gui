#ifndef WS_BR_AGENT_MSG_H
#define WS_BR_AGENT_MSG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ws_br_agent_defs.h"
#include "ws_br_agent_settings.h"

#ifdef __cplusplus
extern "C" {
#endif


// Agent Service Request/Response codes
#define WS_BR_AGENT_MSG_CODE_TOPOLOGY           (0x00000001U)
#define WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS  (0x00000002U)
#define WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS  (0x00000003U)
#define WS_BR_AGENT_MSG_CODE_START_BR           (0x00000004U)
#define WS_BR_AGENT_MSG_CODE_STOP_BR            (0x00000005U)

#define WS_BR_AGENT_MSG_MIN_BUF_SIZE \
  (sizeof(ws_br_agent_msg_code_t) + sizeof(ws_br_agent_msg_len_t))

#define WS_BR_AGENT_MSG_SET_PARAM_MSG_BUF_SIZE \
  (WS_BR_AGENT_MSG_MIN_BUF_SIZE + sizeof(ws_br_agent_msg_settings_payload_t))

typedef uint32_t ws_br_agent_msg_code_t;
typedef uint32_t ws_br_agent_msg_len_t;
typedef ws_br_agent_settings_t ws_br_agent_msg_settings_payload_t;

/// Packet structure:
/// [msg code 4 byte] [payload len 4 byte] [payload data n byte]
typedef struct ws_br_agent_msg {
  ws_br_agent_msg_code_t msg_code;
  ws_br_agent_msg_len_t payload_len;
  uint8_t *payload;
} ws_br_agent_msg_t;

uint8_t *ws_br_agent_msg_build_buf(const ws_br_agent_msg_t * const msg, size_t * const buf_size);

ws_br_agent_msg_t *ws_br_agent_msg_parse_buf(const uint8_t * const buf, const size_t buf_size);

void ws_br_agent_msg_free(ws_br_agent_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_MSG_H