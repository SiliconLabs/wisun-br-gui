#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_srv.h"

#define DISPACH_DELAY_US 1000UL

pthread_t clnt_thr;
volatile sig_atomic_t thread_stop;

static void clnt_thr_fnc(void *arg);
static void sigint_hnd(int signum);

int32_t ws_br_agent_clnt_init(void)
{
  if (pthread_create(&clnt_thr, NULL, (void *)clnt_thr_fnc, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }
  return WS_BR_AGENT_RET_OK;
}

static void clnt_thr_fnc(void *arg)
{
  (void) arg;
  ws_br_agent_log_warn("Client thread started");

  while (!thread_stop) {
    usleep(DISPACH_DELAY_US);
  }
}