#ifndef WS_BR_AGENT_CLNT_H
#define WS_BR_AGENT_CLNT_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

extern pthread_t clnt_thr;

/**
 * \brief Initialize the Wi-SUN SoC Border Router Agent client module.
 */
int32_t ws_br_agent_clnt_init(void);
int32_t ws_br_agent_clnt_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_CLNT_H