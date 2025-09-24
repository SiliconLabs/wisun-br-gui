/***************************************************************************//**
 * @file ws_br_agent_utils.h
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

#ifndef WS_BR_AGENT_UTILS_H
#define WS_BR_AGENT_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_settings.h"

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * @brief Print the application banner to standard output.
 * @details Displays version and copyright information.
 */
void ws_br_agent_utils_print_app_banner(void);

/**
 * @brief Get a string representation of a request code.
 * @param[in] req_code The request code to convert to string.
 * @return Pointer to a string describing the request code.
 */
const char *ws_br_agent_utils_get_req_code_str(const ws_br_agent_msg_code_t req_code);

/**
 * @brief Print the contents of a message structure.
 * @param[in] msg Pointer to the message structure to print.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_utils_print_msg(const ws_br_agent_msg_t * const msg);

/**
 * @brief Print the contents of a host settings structure.
 * @param[in] settings Pointer to the settings structure to print.
 */
void ws_br_agent_utils_print_host_settings(const ws_br_agent_settings_t * const settings);

#if defined(__cplusplus)
}
#endif

#endif // WS_BR_AGENT_UTILS_H