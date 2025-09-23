#ifndef WS_BR_AGENT_DBUS_H
#define WS_BR_AGENT_DBUS_H

#include <pthread.h>
#include "ws_br_agent_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Wi-SUN SoC Border Router Agent D-Bus module.
 * @details This function starts a thread to handle D-Bus communication.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_dbus_init(void);

/**
 * @brief Deinitialize the Wi-SUN SoC Border Router Agent D-Bus module.
 * @details This function stops the D-Bus thread and cleans up resources.
 */
void ws_br_agent_dbus_deinit(void);

/**
 * @brief Notify D-Bus clients that the topology has changed.
 * @details This function emits a signal indicating that the RoutingGraph property has changed.
 * @return WS_BR_AGENT_RET_OK on success, error code otherwise.
 */
ws_br_agent_ret_t ws_br_agent_dbus_notify_topology_changed(void);

#if defined(__cplusplus)
}
#endif

#endif // WS_BR_AGENT_DBUS_H