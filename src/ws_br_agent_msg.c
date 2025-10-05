/***************************************************************************//**
 * @file ws_br_agent_msg.c
 * @brief Message protocol definitions for Wi-SUN SoC Border Router Agent
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


#include <stdlib.h>
#include <string.h>
#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_msg.h"

#define __add_msg_code_and_len_to_buf(ptr, msg)       \
  do {                                                \
    *(uint32_t *)ptr = htonl(msg->msg_code);          \
    ptr += sizeof(ws_br_agent_msg_raw_code_t);        \
    *(uint32_t *)ptr = htonl(msg->payload_len);       \
    ptr += sizeof(ws_br_agent_msg_len_t);             \
  } while(0)

uint8_t *ws_br_agent_msg_build_buf(const ws_br_agent_msg_t * const msg, size_t *buf_size)
{
  uint8_t *ptr = NULL;
  uint8_t *start_ptr = NULL;
  ws_br_agent_msg_settings_payload_t settings_payload = { 0U };

  if (msg == NULL || buf_size ==NULL) {
    return NULL;
  }

  switch(msg->msg_code) {
    case WS_BR_AGENT_MSG_CODE_TOPOLOGY:
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
    case WS_BR_AGENT_MSG_CODE_START_BR:
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
      start_ptr = malloc(WS_BR_AGENT_MSG_MIN_BUF_SIZE);
      if (start_ptr == NULL) {
        ws_br_agent_log_error("Build message error: Memory allocation failed\n");
        return NULL;
      }
      ptr = start_ptr;
      __add_msg_code_and_len_to_buf(ptr, msg);
      break;

    /// Parameter config
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      start_ptr = malloc(WS_BR_AGENT_MSG_SET_PARAM_MSG_BUF_SIZE);
      if (start_ptr == NULL) {
        ws_br_agent_log_error("Build message error: Memory allocation failed\n");
        return NULL;
      }
      ptr = start_ptr;
      __add_msg_code_and_len_to_buf(ptr, msg);
      (void) ws_br_agent_soc_host_get_settings(&settings_payload);
      memcpy((uint8_t *)ptr, &settings_payload, sizeof(ws_br_agent_msg_settings_payload_t));
      ptr += sizeof(ws_br_agent_msg_settings_payload_t);
      break;

    default:
      ws_br_agent_log_error("Build message error: Unsupported request code (0x%2x)\n", msg->msg_code);
      return NULL;
  }
  
  *buf_size = (size_t)(ptr - start_ptr);
  return start_ptr;
}

ws_br_agent_msg_t *ws_br_agent_msg_parse_buf(const uint8_t * const buf, const size_t buf_size)
{
  ws_br_agent_msg_t *msg = NULL;
  uint32_t *ptr = (uint32_t *)buf;

  if (buf == NULL || buf_size < (WS_BR_AGENT_MSG_MIN_BUF_SIZE)) {
    return NULL;
  }

  switch(ntohl(*ptr)) {
    case WS_BR_AGENT_MSG_CODE_TOPOLOGY:
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
    case WS_BR_AGENT_MSG_CODE_START_BR:
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      msg = (ws_br_agent_msg_t *)malloc(sizeof(ws_br_agent_msg_t));
      if (msg == NULL) {
        ws_br_agent_log_error("Parse message error: Memory allocation failed\n");
        return NULL;
      }
      msg->msg_code = ntohl(*ptr);
      ptr++;
      msg->payload_len = ntohl(*ptr);
      ptr++;
      if (msg->payload_len > 0) {
        if (buf_size < (WS_BR_AGENT_MSG_MIN_BUF_SIZE + msg->payload_len)) {
          ws_br_agent_log_error("Parse message error: Invalid payload length\n");
          free(msg);
          return NULL;
        }
        msg->payload = (uint8_t *)malloc(msg->payload_len);
        if (msg->payload == NULL) {
          ws_br_agent_log_error("Parse message error: Memory allocation failed\n");
          free(msg);
          return NULL;
        }
        memcpy(msg->payload, ptr, msg->payload_len);
      } else {
        msg->payload = NULL;
      }
      break;
    default:
      ws_br_agent_log_error("Build message error: Unsupported request code (0x%2x)\n", ntohl(*ptr));
      return NULL;
  }

  return msg;
}

void ws_br_agent_msg_free(ws_br_agent_msg_t *msg)
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