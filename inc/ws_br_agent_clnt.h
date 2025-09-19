#ifndef WS_BR_AGENT_CLNT_H
#define WS_BR_AGENT_CLNT_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include "ws_br_agent_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (*ws_br_agent_clnt_process_resp_cb_t)(const ws_br_agent_msg_t * const msg);

int32_t ws_br_agent_clnt_send_req(const uint8_t req_code, ws_br_agent_clnt_process_resp_cb_t resp_cb);
int32_t ws_br_agent_clnt_set_remote_addr(const char *addr);


#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_CLNT_H