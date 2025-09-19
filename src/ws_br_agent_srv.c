#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_srv.h"

#define DISPACH_DELAY_US 1000UL

pthread_t srv_thr;
volatile sig_atomic_t thread_stop;

static void srv_thr_fnc(void *arg);
static void sigint_hnd(int signum);

ws_br_agent_ret_t ws_br_agent_srv_init(void)
{
  struct sigaction sa;
  sa.sa_handler = sigint_hnd;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  if (pthread_create(&srv_thr, NULL, (void *)srv_thr_fnc, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }
  return WS_BR_AGENT_RET_OK;
}

static void srv_thr_fnc(void *arg)
{
  (void) arg;
  ws_br_agent_log_warn("Server thread started");

  while (!thread_stop) {
    usleep(DISPACH_DELAY_US);
  }
}

static void sigint_hnd(int signum) 
{
  (void) signum;
  thread_stop = 1;
  ws_br_agent_log_warn("Stop application...");
}