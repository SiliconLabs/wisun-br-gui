/***************************************************************************//**
 * @file ws_br_agent_utils.c
 * @brief Utility functions for Wi-SUN SoC Border Router Agent
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_utils.h"

#define MAX_LINE_BUF_SIZE (16U * 5U + 2U)

#if WS_BR_AGENT_SETTINGS_HAVE_KEYS
static void print_4x16_keys(const uint8_t keys[4][16]);
#endif
static const char *phy_type2str(const uint32_t type);

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
  switch (req_code) {
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
const char *ws_br_agent_utils_get_net_size_str(const uint32_t nw_size)
{
  switch (nw_size) {
    case 0U:
      return "Small";
    case 1U:
      return "Medium";
    case 2U:
      return "Large";
    case 3U:
      return "Extra Large";
    default:
      return "Unknown";
  }
}

const char *ws_br_agent_utils_get_reg_domain_str(const uint8_t domain)
{
  switch (domain) {
    case 0: return "WW"; // World wide
    case 1: return "NA"; // North America
    case 2: return "JP"; // Japan
    case 3: return "EU"; // European Union
    case 4: return "CN"; // China
    case 5: return "IN"; // India
    case 6: return "MX"; // Mexico
    case 7: return "BZ"; // Brazil
    case 8: return "AZ/NZ"; // Australia/New Zealand
    case 9: return "KR"; // Korea
    case 10: return "PH"; // Philippines
    case 11: return "MY"; // Malaysia
    case 12: return "HK"; // Hong Kong
    case 13: return "SG"; // Singapore
    case 14: return "TH"; // Thailand
    case 15: return "VN"; // Vietnam
    default: return "Unknown";
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
  ws_br_agent_log_info("Network size: 0x%02x\n", settings->network_size);
  ws_br_agent_log_info("TX power: %d ddBm\n", settings->tx_power_ddbm);

  // Only prints FAN 1.1 (almost always we use that)
  ws_br_agent_log_info("PHY: %s\n", phy_type2str(settings->phy.type));
  if (settings->phy.type == WS_BR_AGENT_PHY_CONFIG_FAN11) {
    ws_br_agent_log_info(" (Regulatory domain: 0x%02x, Channel plan ID: 0x%02x, PHY mode ID: 0x%02x)\n",
                         settings->phy.config.fan11.reg_domain,
                         settings->phy.config.fan11.chan_plan_id,
                         settings->phy.config.fan11.phy_mode_id);

  }
#if WS_BR_AGENT_SETTINGS_HAVE_KEYS
  ws_br_agent_log_info("GAKs:\n");
  print_4x16_keys(settings->gaks);
  ws_br_agent_log_info("GTKs:\n");
  print_4x16_keys(settings->gtks);
#endif
}

#if WS_BR_AGENT_SETTINGS_HAVE_KEYS
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
#endif


static const char *phy_type2str(const uint32_t type)
{
  switch(type) {
    /// FAN1.0 PHY configuration
    case WS_BR_AGENT_PHY_CONFIG_FAN10:
      return "FAN1.0";  
    /// FAN1.1 PHY configuration
    case WS_BR_AGENT_PHY_CONFIG_FAN11:
      return "FAN1.1";
    /// Explicit PHY configuration
    case WS_BR_AGENT_PHY_CONFIG_EXPLICIT:
      return "EXPLICIT";
    /// Explicit RAIL configuration
    case WS_BR_AGENT_PHY_CONFIG_IDS:
      return "IDS";
    /// Custom FSK PHY configuration
    case WS_BR_AGENT_PHY_CONFIG_CUSTOM_FSK:
      return "CUSTOM_FSK";
    /// Custom OFDM PHY configuration
    case WS_BR_AGENT_PHY_CONFIG_CUSTOM_OFDM:
      return "CUSTOM_OFDM";
    /// Custom OQPSK PHY configuration
    case WS_BR_AGENT_PHY_CONFIG_CUSTOM_OQPSK:
      return "CUSTOM_OQPSK";
    default:
      return "UNKNOWN";
  }
}
