#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_clnt.h"

pthread_t clnt_thr;

static void clnt_thr_fnc(void *arg);

int32_t ws_br_agent_clnt_init(void)
{
  if (pthread_create(&clnt_thr, NULL, (void *)clnt_thr_fnc, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }

  return WS_BR_AGENT_RET_OK;
}

int32_t ws_br_agent_clnt_deinit(void)
{
  return WS_BR_AGENT_RET_ERR;
}


static void clnt_thr_fnc(void *arg)
{
  (void) arg;
  ws_br_agent_log_warn("Client thread started.");
  while (1) {
    usleep(1000000);
    ws_br_agent_log_info("Client thread is running...");
  }
}