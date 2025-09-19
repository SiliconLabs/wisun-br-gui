#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_utils.h"

void ws_br_agent_print_app_banner(void)
{
  const int banner_width = 60;
  int len_app_name = sizeof(WS_BR_AGENT_APP_NAME) - 1;
  int len_version = sizeof(WS_BR_AGENT_VERSION) - 1;
  int len_copyright = sizeof(WS_BR_AGENT_APP_COPYRIGHT_STR) - 1;
  int padding_left = 0;
  int padding_right = 0;
  
  for (int i = 0; i < banner_width; i++) ws_br_agent_app_print("*");
  ws_br_agent_app_print("\n");
  padding_left = (banner_width - 2 - len_app_name) / 2;
  padding_right = banner_width - 2 - len_app_name - padding_left;
  ws_br_agent_app_print("*%*c" WS_BR_AGENT_APP_NAME "%*c*\n", padding_left, ' ', padding_right, ' ');
  padding_left = (banner_width - 3 - len_version) / 2;
  padding_right = banner_width - 3 - len_version - padding_left;
  ws_br_agent_app_print("*%*cv" WS_BR_AGENT_VERSION "%*c*\n", padding_left, ' ', padding_right, ' ');
  padding_left = (banner_width - 2 - len_copyright) / 2;
  padding_right = banner_width - 2 - len_copyright - padding_left;
  ws_br_agent_app_print("*%*c" WS_BR_AGENT_APP_COPYRIGHT_STR "%*c*\n", padding_left, ' ', padding_right, ' ');
  for (int i = 0; i < banner_width; i++) ws_br_agent_app_print("*");
  ws_br_agent_app_print("\n");
}

const char *ws_br_utils_get_req_code_str(const uint32_t req_code)
{
  switch(req_code) {
    case WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY:
      return "GET_TOPOLOGY";
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
      return "GET_CONFIG_PARAMS";
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      return "SET_CONFIG_PARAMS";
    case WS_BR_AGENT_MSG_CODE_START_BR:
      return "START_BR";
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
      return "STOP_BR";
    default:
      return "UNKNOWN";
  }
}

int32_t ws_br_agent_utils_print_msg(const ws_br_agent_msg_t * const msg)
{
  if (msg == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Msg code: %04x\n", msg->msg_code);
  ws_br_agent_log_info("Payload len: %u\n", msg->payload_len);

  if (!msg->payload_len) {
    return WS_BR_AGENT_RET_OK;
  }

  ws_br_agent_log_info("Payload data:");
  for (size_t i = 0; i < msg->payload_len; i++) {
    if (i % 16 == 0) {
      ws_br_agent_log_info("\n%04x: ", (unsigned int)i);  
    }
    ws_br_agent_log_info("%02x ", msg->payload[i]);
  }
  ws_br_agent_log_info("\n");
  
  return WS_BR_AGENT_RET_OK;
}