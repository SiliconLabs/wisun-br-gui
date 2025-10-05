/***************************************************************************//**
 * @file ws_br_agent_settings.c
 * @brief Soc Border Router settings
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
#include <errno.h>
#include "ws_br_agent_settings.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_utils.h"

static int parse_escape_sequences(char *out, const char *in, size_t max_len);
static ws_br_agent_ret_t parse_config_line(const char *line, ws_br_agent_settings_t *settings);

ws_br_agent_ret_t ws_br_agent_settings_load_config(const char * conf_file, 
                                                   ws_br_agent_settings_t *settings)
{
  FILE *file = NULL;
  char line[512];
  int line_number = 0;

  if (conf_file == NULL || settings == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  file = fopen(conf_file, "r");
  if (file == NULL) {
    ws_br_agent_log_error("Failed to open config file: %s\n", conf_file);
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Loading configuration from: %s\n", conf_file);

  
  // Read configuration file line by line
  while (fgets(line, sizeof(line), file) != NULL) {
    line_number++;

    if (parse_config_line(line, settings) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_warn("Failed to parse line %d: %s\n", line_number, line);
    }
  }
  
  // Check for file read errors
  if (ferror(file)) {
    ws_br_agent_log_error("Error reading config file: %s\n", conf_file);
    fclose(file);
    return WS_BR_AGENT_RET_ERR;
  }
  
  fclose(file);

  ws_br_agent_log_info("Configuration loaded successfully (%d lines processed)\n", line_number);
  return WS_BR_AGENT_RET_OK;
}

static int parse_escape_sequences(char *out, const char *in, size_t max_len)
{
  char tmp[3], conv, *end_ptr;
  int i, j;

  if (!max_len) {
    return -EINVAL;
  }
  
  j = 0;

  for (i = 0; in[i]; ) {
    if (j >= max_len - 1) {
      out[j] = '\0';
      return -ERANGE;
    }
    if (in[i] == '\\') {
      if (in[i + 1] != 'x')
          return -1;
      tmp[0] = in[i + 2];
      tmp[1] = in[i + 3];
      tmp[2] = '\0';
      conv = strtol(tmp, &end_ptr, 16);
      out[j++] = conv;
      if (*end_ptr || !conv) {
          out[j] = '\0';
          return -EINVAL;
      }
      i += 4;
    } else {
      out[j++] = in[i++];
    }
  }
  out[j++] = '\0';
  return 0;
}

static ws_br_agent_ret_t parse_config_line(const char *line, ws_br_agent_settings_t *settings)
{
  char key[256], value[256];
  char *trimmed_line;
  char *comment_pos;
  int tmp_val;
  
  if (line == NULL || settings == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  // Create a mutable copy of the line
  trimmed_line = strdup(line);
  if (trimmed_line == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  // Remove trailing newline and carriage return
  size_t len = strlen(trimmed_line);
  while (len > 0 && (trimmed_line[len-1] == '\n' || trimmed_line[len-1] == '\r')) {
    trimmed_line[--len] = '\0';
  }
  
  // Skip empty lines and comments
  if (len == 0 || trimmed_line[0] == '#' || trimmed_line[0] == ';') {
    free(trimmed_line);
    return WS_BR_AGENT_RET_OK;
  }
  
  // Remove inline comments
  comment_pos = strchr(trimmed_line, '#');
  if (comment_pos != NULL) {
    *comment_pos = '\0';
  }
  comment_pos = strchr(trimmed_line, ';');
  if (comment_pos != NULL) {
    *comment_pos = '\0';
  }
  
  // Parse key=value pair
  if (sscanf(trimmed_line, "%255[^=]=%255s", key, value) != 2) {
    free(trimmed_line);
    return WS_BR_AGENT_RET_OK; // Skip malformed lines
  }
  
  // Trim whitespace from key
  char *key_start = key;
  while (*key_start == ' ' || *key_start == '\t') key_start++;
  char *key_end = key_start + strlen(key_start) - 1;
  while (key_end > key_start && (*key_end == ' ' || *key_end == '\t')) {
    *key_end-- = '\0';
  }
  
  // Parse configuration values
  if (strcmp(key_start, "network_name") == 0) {
    if (parse_escape_sequences(settings->network_name, value, 
                           WS_BR_AGENT_NETWORK_NAME_SIZE + 1) == 0) {
      ws_br_agent_log_debug("Configure network name: %s\n", settings->network_name);
    } else {
      ws_br_agent_log_warn("Invalid network name");
    }

  } else if (strcmp(key_start, "size") == 0) {
    if (ws_br_agent_utils_str_to_val(value, ws_br_agent_nw_size_strs, 
                                     &tmp_val) == WS_BR_AGENT_RET_OK) {
      settings->network_size = (uint8_t)tmp_val;
      ws_br_agent_log_debug("Configure network size: %s (%u)\n", value, settings->network_size);
    } else {
      ws_br_agent_log_warn("Unknown network size: %s\n", value);
    }

  } else if (strcmp(key_start, "tx_power_ddbm") == 0) {
    settings->tx_power_ddbm = (int16_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure tx power (ddBm): %d\n", settings->tx_power_ddbm);

  } else if (strcmp(key_start, "uc_dwell_interval_ms") == 0) {
    settings->uc_dwell_interval_ms = (uint8_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure uc dwell interval (ms): %u\n", settings->uc_dwell_interval_ms);

  } else if (strcmp(key_start, "bc_interval_ms") == 0) {
    settings->bc_interval_ms = (uint32_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure bc interval (ms): %u\n", settings->bc_interval_ms);

  } else if (strcmp(key_start, "bc_dwell_interval_ms") == 0) {
    settings->bc_dwell_interval_ms = (uint8_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure bc dwell interval (ms): %u\n", settings->bc_dwell_interval_ms);

  } else if (strcmp(key_start, "allowed_channels") == 0) {
    if (parse_escape_sequences(settings->allowed_channels, value, 
                           WS_BR_AGENT_SETTINGS_STR_DATA_MAX_LENGTH + 1) == 0) {
      ws_br_agent_log_debug("Configure allowed channels: %s\n", settings->allowed_channels);
    } else {
      ws_br_agent_log_warn("Invalid allowed channels string");
    }

  } else if (strcmp(key_start, "ipv6_prefix") == 0) {
    strncpy(settings->ipv6_prefix, value, WS_BR_AGENT_IPV6_PREFIX_SIZE);
    settings->ipv6_prefix[WS_BR_AGENT_IPV6_PREFIX_SIZE - 1] = '\0';
    ws_br_agent_log_debug("Configure IPv6 prefix: %s\n", settings->ipv6_prefix);
    
  } else if (strcmp(key_start, "max_neighbor_count") == 0) {
    settings->max_neighbor_count = (uint8_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure max neighbor count: %u\n", settings->max_neighbor_count);

  } else if (strcmp(key_start, "max_child_count") == 0) {
    settings->max_child_count = (uint8_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure max child count: %u\n", settings->max_child_count);

  } else if (strcmp(key_start, "max_security_neighbor_count") == 0) {
    settings->max_security_neighbor_count = (uint16_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure max security neighbor count: %u\n", settings->max_security_neighbor_count);

  } else if (strcmp(key_start, "keychain") == 0) {
    if (ws_br_agent_utils_str_to_val(value, ws_br_agent_keychain_strs, 
                                     &tmp_val) == WS_BR_AGENT_RET_OK) {
      settings->keychain = (uint8_t)tmp_val;
      ws_br_agent_log_debug("Configure keychain: %s (%u)\n", value, settings->keychain);
    } else {
      ws_br_agent_log_warn("Unknown keychain: %s\n", value);
    }

  } else if (strcmp(key_start, "keychain_index") == 0) {
    settings->keychain_index = (uint8_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure keychain index: %u\n", settings->keychain_index);

  } else if (strcmp(key_start, "socket_rx_buffer_size") == 0) {
    settings->socket_rx_buffer_size = (uint16_t)strtol(value, NULL, 0);
    ws_br_agent_log_debug("Configure socket RX buffer size: %u\n", settings->socket_rx_buffer_size);

  } else if (strcmp(key_start, "domain") == 0) {
    // Only FAN 1.1 supported currently
    if (settings->phy.type == WS_BR_AGENT_PHY_CONFIG_FAN11 && 
        ws_br_agent_utils_str_to_val(value, ws_br_agent_domains_strs, 
                                     &tmp_val) == WS_BR_AGENT_RET_OK) {
      settings->phy.config.fan11.reg_domain = (uint8_t)tmp_val;
      ws_br_agent_log_debug("Configure regulatory domain: %s (%u)\n", value, settings->phy.config.fan11.reg_domain);
    }
  
  } else if (strcmp(key_start, "chan_plan_id") == 0) {
    // Only FAN 1.1 supported currently
    if (settings->phy.type == WS_BR_AGENT_PHY_CONFIG_FAN11) {
      settings->phy.config.fan11.chan_plan_id = (uint8_t)strtol(value, NULL, 0);
      ws_br_agent_log_debug("Configure channel plan ID: %u\n", settings->phy.config.fan11.chan_plan_id);
    }
  
  } else if (strcmp(key_start, "phy_mode_id") == 0) {
    // Only FAN 1.1 supported currently
    if (settings->phy.type == WS_BR_AGENT_PHY_CONFIG_FAN11) {
      settings->phy.config.fan11.phy_mode_id = (uint8_t)strtol(value, NULL, 0);
      ws_br_agent_log_debug("Configure PHY mode ID: %u\n", settings->phy.config.fan11.phy_mode_id);
    }
  } else {
    ws_br_agent_log_warn("Unknown config parameter: %s\n", key_start);
  }
  
  free(trimmed_line);
  return WS_BR_AGENT_RET_OK;
}