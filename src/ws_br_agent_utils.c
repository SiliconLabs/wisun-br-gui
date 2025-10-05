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
#include "ws_br_agent_settings.h"

#define MAX_LINE_BUF_SIZE (16U * 5U + 2U)

#if WS_BR_AGENT_SETTINGS_HAVE_KEYS
static void print_4x16_keys(const uint8_t keys[4][16]);
#endif

const ws_br_agent_name_value_t ws_br_agent_nw_size_strs[] = {
    { "CERT",   WS_BR_AGENT_NETWORK_SIZE_CERTIFICATION },
    { "SMALL",  WS_BR_AGENT_NETWORK_SIZE_SMALL },
    { "S",      WS_BR_AGENT_NETWORK_SIZE_SMALL },
    { "MEDIUM", WS_BR_AGENT_NETWORK_SIZE_MEDIUM },
    { "M",      WS_BR_AGENT_NETWORK_SIZE_MEDIUM },
    { "LARGE",  WS_BR_AGENT_NETWORK_SIZE_LARGE },
    { "L",      WS_BR_AGENT_NETWORK_SIZE_LARGE },
    { "XLARGE", WS_BR_AGENT_NETWORK_SIZE_XLARGE },
    { "XL",     WS_BR_AGENT_NETWORK_SIZE_XLARGE },
    { NULL, 0L }
};

const ws_br_agent_name_value_t ws_br_agent_domains_strs[] = {
    { "WW", WS_BR_AGENT_REG_DOMAIN_WW }, // World wide
    { "NA", WS_BR_AGENT_REG_DOMAIN_NA }, // North America
    { "JP", WS_BR_AGENT_REG_DOMAIN_JP }, // Japan
    { "EU", WS_BR_AGENT_REG_DOMAIN_EU }, // European Union
    { "CN", WS_BR_AGENT_REG_DOMAIN_CN }, // China
    { "IN", WS_BR_AGENT_REG_DOMAIN_IN }, // India
    { "MX", WS_BR_AGENT_REG_DOMAIN_MX }, // Mexico
    { "BZ", WS_BR_AGENT_REG_DOMAIN_BZ }, // Brazil
    { "AZ", WS_BR_AGENT_REG_DOMAIN_AZ }, // Australia
    { "NZ", WS_BR_AGENT_REG_DOMAIN_NZ }, // New Zealand (share its ID with Australia)
    { "KR", WS_BR_AGENT_REG_DOMAIN_KR }, // Korea
    { "PH", WS_BR_AGENT_REG_DOMAIN_PH }, // Philippines
    { "MY", WS_BR_AGENT_REG_DOMAIN_MY }, // Malaysia
    { "HK", WS_BR_AGENT_REG_DOMAIN_HK }, // Hong Kong
    { "SG", WS_BR_AGENT_REG_DOMAIN_SG }, // Singapore
    { "TH", WS_BR_AGENT_REG_DOMAIN_TH }, // Thailand
    { "VN", WS_BR_AGENT_REG_DOMAIN_VN }, // Vietnam
    { NULL, 0L }
};

const ws_br_agent_name_value_t ws_br_agent_keychain_strs[]  = {
  { "AUTO",    WS_BR_AGENT_KEYCHAIN_AUTOMATIC },
  { "BUILTIN", WS_BR_AGENT_KEYCHAIN_BUILTIN },
  { "NVM",     WS_BR_AGENT_KEYCHAIN_NVM },
  { NULL, 0L }
};

const ws_br_agent_name_value_t ws_br_agent_msg_code_strs[] = {
  { "TOPOLOGY",            WS_BR_AGENT_MSG_CODE_TOPOLOGY },
  { "GET_CONFIG_PARAMS",   WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS },
  { "SET_CONFIG_PARAMS",   WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS },
  { "START_BR",            WS_BR_AGENT_MSG_CODE_START_BR },
  { "STOP_BR",             WS_BR_AGENT_MSG_CODE_STOP_BR },
  { NULL, 0L }
};

const ws_br_agent_name_value_t ws_br_agent_phy_type_strs[] = {
  { "FAN1.1",        WS_BR_AGENT_PHY_CONFIG_FAN11 },
  { "FAN1.0",        WS_BR_AGENT_PHY_CONFIG_FAN10 },
  { "Explicit",      WS_BR_AGENT_PHY_CONFIG_EXPLICIT },
  { "Explicit RAIL", WS_BR_AGENT_PHY_CONFIG_IDS },
  { "Custom FSK",    WS_BR_AGENT_PHY_CONFIG_CUSTOM_FSK },
  { "Custom OFDM",   WS_BR_AGENT_PHY_CONFIG_CUSTOM_OFDM },
  { "Custom OQPSK",  WS_BR_AGENT_PHY_CONFIG_CUSTOM_OQPSK },
  { NULL, 0L }
};

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

int32_t ws_br_agent_utils_print_msg(const ws_br_agent_msg_t * const msg)
{
  char *line_buf = NULL;

  if (msg == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_debug("Msg code: %s (0x%08x)\n", 
                       ws_br_agent_utils_val_to_str(msg->msg_code, 
                                                    ws_br_agent_msg_code_strs, 
                                                    "Unknown"), 
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
  ws_br_agent_log_info("PHY: %s\n", ws_br_agent_utils_val_to_str(settings->phy.type, 
                                                                 ws_br_agent_phy_type_strs, 
                                                                 "Unknown"));
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


const char *ws_br_agent_utils_val_to_str(int val, 
                                         const ws_br_agent_name_value_t table[], 
                                         const char *def)
{
  for (int i = 0; table[i].name != NULL; i++) {
    if (val == table[i].val) {
      return table[i].name;
    }
  }
  return def == NULL ? "NULL" : def;
}

ws_br_agent_ret_t ws_br_agent_utils_str_to_val(const char *str, 
                                               const ws_br_agent_name_value_t table[], 
                                               int *res)
{
  if (str == NULL || res == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  for (int i = 0; table[i].name != NULL; ++i) {
    if (!strcmp(str, table[i].name)) {
      *res = table[i].val;
      return WS_BR_AGENT_RET_OK;
    }
  }

  return WS_BR_AGENT_RET_ERR;
}
