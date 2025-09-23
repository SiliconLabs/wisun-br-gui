#ifndef WS_BR_AGENT_DBUS_H
#define WS_BR_AGENT_DBUS_H

#include <pthread.h>
#include "ws_br_agent_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

ws_br_agent_ret_t ws_br_agent_dbus_init(void);
void ws_br_agent_dbus_deinit(void);

#if defined(__cplusplus)
}
#endif

#endif // WS_BR_AGENT_DBUS_H