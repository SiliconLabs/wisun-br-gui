#ifndef WS_BR_AGENT_CLNT_H
#define WS_BR_AGENT_CLNT_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "ws_br_agent_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ws_br_agent_soc_host {
  char remote_addr_str[WS_BR_AGENT_IPV6_ADDR_STR_SIZE];
  struct sockaddr_in6 remote_addr;
  ws_br_agent_settings_t settings;
} ws_br_agent_soc_host_t;

typedef struct __attribute__((aligned(1))) 
ws_br_agent_soc_host_topology_entry {
  /// GUA/ULA of the routed node
  uint8_t target[16];
  /// GUA/ULA of the routed node's preferred parent
  uint8_t preferred[16];
  /// GUA/ULA of the routed node's backup parent. INADDR_ANY if not set.
  uint8_t backup[16];
} ws_br_agent_soc_host_topology_entry_t;

typedef struct ws_br_agent_soc_host_topology {
  uint8_t entry_count;
  ws_br_agent_soc_host_topology_entry_t *entries;
} ws_br_agent_soc_host_topology_t;

typedef ws_br_agent_ret_t (*ws_br_agent_soc_host_process_resp_cb_t)(const ws_br_agent_msg_t * const msg);

ws_br_agent_ret_t ws_br_agent_soc_host_init(void);

ws_br_agent_ret_t ws_br_agent_soc_host_send_req(const ws_br_agent_msg_t * const req_msg, 
                                                ws_br_agent_soc_host_process_resp_cb_t resp_cb);

ws_br_agent_ret_t ws_br_agent_soc_host_set(const char *addr,
                                           const ws_br_agent_settings_t *const settings);

ws_br_agent_ret_t ws_br_agent_soc_host_get(ws_br_agent_soc_host_t * const dst_host);

ws_br_agent_ret_t ws_br_agent_soc_host_set_remote_addr(const struct sockaddr_in6 * const addr);

ws_br_agent_ret_t ws_br_agent_soc_host_get_remote_addr(struct sockaddr_in6 * const addr);

const ws_br_agent_settings_t *ws_br_agent_soc_host_get_default_settings(void);

ws_br_agent_ret_t ws_br_agent_soc_host_set_settings(const ws_br_agent_settings_t * const settings);

ws_br_agent_ret_t ws_br_agent_soc_host_get_settings(ws_br_agent_settings_t * const settings);

ws_br_agent_ret_t ws_br_agent_soc_host_set_topology(const ws_br_agent_soc_host_topology_t *topology);

ws_br_agent_ret_t ws_br_agent_soc_host_get_topology(ws_br_agent_soc_host_topology_t * const topology);

ws_br_agent_ret_t ws_br_agent_soc_host_free_topology(ws_br_agent_soc_host_topology_t * const topology);

#ifdef __cplusplus
}
#endif

#endif // WS_BR_AGENT_CLNT_H