#ifndef WS_BR_AGENT_SRV_H
#define WS_BR_AGENT_SRV_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

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