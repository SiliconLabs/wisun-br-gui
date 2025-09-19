#include <assert.h>
#include <stdlib.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_srv.h"
#include "ws_br_agent_utils.h"


extern pthread_t srv_thr;

int main(int argc, char *argv[]) 
{
  (void) argc;
  (void) argv;

  ws_br_agent_print_app_banner();

  assert(ws_br_agent_soc_host_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_srv_init() == WS_BR_AGENT_RET_OK);

  // TODO remove
  (void) ws_br_agent_soc_host_set("fe80::1", NULL);

  pthread_join(srv_thr, NULL);
  
  return EXIT_SUCCESS;
}