#include <assert.h>
#include <stdlib.h>
#include <signal.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_clnt.h"


static void sigint_hnd(int signum);

int main(int argc, char *argv[]) {
  
  struct sigaction sa;
  sa.sa_handler = sigint_hnd;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  
  ws_br_agent_print_app_banner();
  
  assert(ws_br_agent_clnt_init() == WS_BR_AGENT_RET_OK);

  // TODO remove
  (void) ws_br_agent_clnt_set_remote_addr("fe80::1");

  pthread_join(clnt_thr, NULL);
  
  return EXIT_SUCCESS;
}

static void sigint_hnd(int signum) 
{
  (void) signum;
  thread_stop = 1;
  ws_br_agent_log_warn("Stop application...");
}