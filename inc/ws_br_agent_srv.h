/***************************************************************************//**
 * @file ws_br_agent_srv.h
 * @brief Border Router Agent service
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

#ifndef WS_BR_AGENT_SRV_H
#define WS_BR_AGENT_SRV_H

#include "ws_br_agent_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Wi-SUN SoC Border Router Agent server module.
 * @details Prepares resources and starts the server for handling incoming requests.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_srv_init(void);

/**
 * @brief Deinitialize the Wi-SUN SoC Border Router Agent server module.
 * @details Cleans up resources and stops the server.
 */
void ws_br_agent_srv_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_SRV_H