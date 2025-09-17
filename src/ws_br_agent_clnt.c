#include <pthread.h>
#include <stdlib.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_clnt.h"


static void clnt_thr_fnc(void *arg);

int32_t ws_br_agent_clnt_init(void)
{
  
  return WS_BR_AGENT_RET_OK;
}

int32_t ws_br_agent_clnt_deinit(void)
{
  return WS_BR_AGENT_RET_ERR;
}


static void clnt_thr_fnc(void *arg)
{
  (void) arg;
  while (1) {
    // Client thread functionality goes here
  }
}