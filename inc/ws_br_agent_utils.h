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

#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_settings.h"

#if defined(__cplusplus)
extern "C" {
#endif

/// Name-value pair structure for Wi-SUN SoC Border Router Agent.
typedef struct ws_br_agent_name_value {
  /// Name
  char *name;
  ///  Value
  int val;
} ws_br_agent_name_value_t;

/// Wi-SUN Network size string table
extern const ws_br_agent_name_value_t ws_br_agent_nw_size_strs[];

/// Wi-SUN Domain string table
extern const ws_br_agent_name_value_t ws_br_agent_domains_strs[];

/// Wi-SUN Keychain string table
extern const ws_br_agent_name_value_t ws_br_agent_keychain_strs[];

/// BR Agent message code string table
extern const ws_br_agent_name_value_t ws_br_agent_msg_code_strs[];

/// Wi-SUN PHY types string table
extern const ws_br_agent_name_value_t ws_br_agent_phy_type_strs[];

/**
 * @brief Print the application banner to standard output.
 * @details Displays version and copyright information.
 */
void ws_br_agent_utils_print_app_banner(void);

/**
 * @brief Print the help message to standard output.
 * @details Displays usage information and available command-line options.
 */
void ws_br_agent_utils_print_help(void);

/**
 * @brief Print version information to standard output.
 * @details Displays version information.
 */
void ws_br_agent_utils_print_version(void);

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

/**
 * @brief Convert a value to its corresponding string using a name-value table.
 * @param[in] val The value to convert.
 * @param[in] table The name-value table to use for conversion.
 * @param[in] def The default string to return if the value is not found in the table.
 * @return Pointer to the corresponding string, or the default string if not found.
 */
const char *ws_br_agent_utils_val_to_str(int val, 
                                         const ws_br_agent_name_value_t table[], 
                                         const char *def);

/**
 * @brief Convert a string to its corresponding value using a name-value table.
 * @param[in] str The string to convert.
 * @param[in] table The name-value table to use for conversion.
 * @param[out] res Pointer to an integer to store the resulting value.
 * @return The corresponding value, or -1 if the string is not found in the table.
 */
ws_br_agent_ret_t ws_br_agent_utils_str_to_val(const char *str, 
                                               const ws_br_agent_name_value_t table[], 
                                               int *res);

#if defined(__cplusplus)
}
#endif

#endif // WS_BR_AGENT_UTILS_H