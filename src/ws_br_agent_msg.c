#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"


#define __add_msg_code_and_len_to_buf(ptr, msg)       \
  do {                                                \
    *(uint32_t *)ptr = htonl(msg->msg_code);         \
    ptr += sizeof(ws_br_agent_msg_code_t);            \
    *(uint32_t *)ptr = htonl(msg->payload_len);      \
    ptr += sizeof(ws_br_agent_msg_len_t);             \
  } while(0)

int32_t ws_br_msg_build_buf(ws_br_agent_msg_t * const msg, size_t *buf_size)
{
  uint8_t *ptr = NULL;
  
  // TODO Replace with settings API
  ws_br_agent_param_payload_t params = {
    .network_name = "Wi-SUN_Network",
    .regulatory_domain = 1,
    .network_size = 0,
    .chan_plan_id = 0,
    .phy_mode_id = 0,
    .tx_power_ddbm = 0,
    .pan_id = 0x1234,
    .gaks = { {0}, {0}, {0}, {0} },
    .gtks = { {0}, {0}, {0}, {0} }
  };

  if (msg == NULL || buf_size ==NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  switch(msg->msg_code) {
    case WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY:
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
    case WS_BR_AGENT_MSG_CODE_START_BR:
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
      ptr = malloc(WS_BR_AGENT_MSG_DEFAULT_BUF_SIZE);
      if (ptr == NULL) {
        ws_br_agent_log_err("Build message error: Memory allocation failed");
        return WS_BR_AGENT_RET_ERR;
      }
      __add_msg_code_and_len_to_buf(ptr, msg);
      break;

    /// Parameter config
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      ptr = malloc(WS_BR_AGENT_MSG_SET_PARAM_MSG_BUF_SIZE);
      if (ptr == NULL) {
        ws_br_agent_log_err("Build message error: Memory allocation failed");
        return WS_BR_AGENT_RET_ERR;
      }
      __add_msg_code_and_len_to_buf(ptr, msg);
      memcpy((uint8_t *)ptr, &params, sizeof(ws_br_agent_param_payload_t));
      ptr += sizeof(ws_br_agent_param_payload_t);
      break;

    default:
      ws_br_agent_log_err("Build message error: Unsupported request code (0x%2x)", req_code);
      return WS_BR_AGENT_RET_ERR;
  }
  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_msg_t *ws_br_msg_parse_buf(const uint8_t * const buf, const size_t buf_size)
{
  ws_br_agent_msg_t *msg = NULL;
  uint32_t *ptr = (uint32_t *)buf;

  if (buf == NULL || buf_size < (WS_BR_AGENT_MSG_DEFAULT_BUF_SIZE)) {
    return NULL;
  }

  switch(ntohl(*ptr)) {
    case WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY:
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
    case WS_BR_AGENT_MSG_CODE_START_BR:
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      msg = (ws_br_agent_msg_t *)malloc(sizeof(ws_br_agent_msg_t));
      if (msg == NULL) {
        ws_br_agent_log_err("Parse message error: Memory allocation failed");
        return NULL;
      }
      msg->msg_code = ntohl(*ptr);
      ptr++;
      msg->payload_len = ntohl(*ptr);
      ptr++;
      if (msg->payload_len > 0) {
        if (buf_size < (WS_BR_AGENT_MSG_DEFAULT_BUF_SIZE + msg->payload_len)) {
          ws_br_agent_log_err("Parse message error: Invalid payload length");
          free(msg);
          return NULL;
        }
        msg->payload = (uint8_t *)malloc(msg->payload_len);
        if (msg->payload == NULL) {
          ws_br_agent_log_err("Parse message error: Memory allocation failed");
          free(msg);
          return NULL;
        }
        memcpy(msg->payload, ptr, msg->payload_len);
      } else {
        msg->payload = NULL;
      }
      break;
    default:
      ws_br_agent_log_err("Build message error: Unsupported request code (0x%2x)", ntohl(*ptr));
      return WS_BR_AGENT_RET_ERR;
  }

  return msg;
}

void ws_br_msg_free(ws_br_agent_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }
  // Free payload if allocated
  if (msg->payload != NULL) {
    free(msg->payload);
  }
  // Free message structure
  free(msg);
}