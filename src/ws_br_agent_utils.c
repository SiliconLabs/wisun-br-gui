#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_utils.h"


#define MAX_LINE_BUF_SIZE (16U * 5U + 2U)

static void print_4x16_keys(const uint8_t keys[4][16]);

void ws_br_agent_utils_print_app_banner(void)
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

const char *ws_br_agent_utils_get_req_code_str(const ws_br_agent_msg_code_t req_code)
{
  switch(req_code) {
    case WS_BR_AGENT_MSG_CODE_TOPOLOGY:
      return "TOPOLOGY";
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
  char *line_buf = NULL;

  if (msg == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_debug("Msg code: %s (0x%08x)\n", 
                       ws_br_agent_utils_get_req_code_str(msg->msg_code), 
                       msg->msg_code);
  ws_br_agent_log_debug("Payload len: %u\n", msg->payload_len);

  if (!msg->payload_len) {
    return WS_BR_AGENT_RET_OK;
  }

  line_buf = (char *)malloc(MAX_LINE_BUF_SIZE);

  ws_br_agent_log_debug("Payload data:\n");
  for (size_t i = 0, cnt = 0; i < msg->payload_len; i++) {
    snprintf(&line_buf[cnt], MAX_LINE_BUF_SIZE - cnt, " 0x%02x", msg->payload[i]);
    cnt += 5;

    if (((i + 1) % 16 == 0) || (i + 1 == msg->payload_len)) {
      ws_br_agent_log_debug("%s\n", line_buf);
      cnt = 0;
    }
  }

  free(line_buf);

  return WS_BR_AGENT_RET_OK;
}

void ws_br_agent_utils_print_host_settings(const ws_br_agent_settings_t * const settings)
{
  
  ws_br_agent_log_info("Network name: %s\n", settings->network_name);
  ws_br_agent_log_info("Regulatory domain: 0x%02x\n", settings->regulatory_domain);
  ws_br_agent_log_info("Network size: 0x%02x\n", settings->network_size);
  ws_br_agent_log_info("TX power: %d ddBm\n", settings->tx_power_ddbm);
  ws_br_agent_log_info("PAN ID: 0x%04x\n", settings->pan_id);
  ws_br_agent_log_info("GAKs:\n");
  print_4x16_keys(settings->gaks);
  ws_br_agent_log_info("GTKs:\n");
  print_4x16_keys(settings->gtks);
}

static void print_4x16_keys(const uint8_t keys[4][16])
{
  char *line_buf = NULL;
  
  for (uint8_t i = 0; i < 4; ++i) {
    if (line_buf != NULL) {
      free(line_buf);
      line_buf = NULL;
    }
    line_buf = (char *) malloc(MAX_LINE_BUF_SIZE);

    for (uint8_t j = 0; j < 16; ++j) {
      snprintf(&line_buf[j * 5], MAX_LINE_BUF_SIZE - j * 5, 
               " 0x%02x", keys[i][j]);
    }

    line_buf[MAX_LINE_BUF_SIZE - 1] = 0;
    
    ws_br_agent_log_info("%s\n", line_buf);
  }

  free(line_buf);
}
