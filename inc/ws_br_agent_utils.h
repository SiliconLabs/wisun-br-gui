#ifndef WS_BR_AGENT_UTILS_H
#define WS_BR_AGENT_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"

#if defined(__cplusplus)
extern "C" {
#endif


void ws_br_agent_print_app_banner(void);
const char *ws_br_utils_get_req_code_str(const uint32_t req_code);
ws_br_agent_ret_t ws_br_agent_utils_print_msg(const ws_br_agent_msg_t * const msg);

#if defined(__cplusplus)
}
#endif

#endif // WS_BR_AGENT_UTILS_H