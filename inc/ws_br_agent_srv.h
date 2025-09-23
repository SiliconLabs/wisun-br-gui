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
 * \brief Initialize the Wi-SUN SoC Border Router Agent client module.
 */
ws_br_agent_ret_t ws_br_agent_srv_init(void);

void ws_br_agent_srv_deinit(void);


#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_SRV_H