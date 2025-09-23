#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_srv.h"
#include "ws_br_agent_utils.h"
#include "ws_br_agent_dbus.h"

static void sigint_hnd(int signum);
static volatile sig_atomic_t main_thread_stop = 0;

int main(int argc, char *argv[]) 
{
  struct sigaction sa;

  (void) argc;
  (void) argv;
  
  sa.sa_handler = sigint_hnd;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);


  ws_br_agent_utils_print_app_banner();

  assert(ws_br_agent_soc_host_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_srv_init() == WS_BR_AGENT_RET_OK);
  assert(ws_br_agent_dbus_init() == WS_BR_AGENT_RET_OK);
  
  while(!main_thread_stop) {
    usleep(1000000UL); // 1 second  
  }

  return EXIT_SUCCESS;
}

static void sigint_hnd(int signum)
{
  (void)signum;
  ws_br_agent_srv_deinit();
  ws_br_agent_dbus_deinit();
  ws_br_agent_log_warn("Stop application...\n");
  main_thread_stop = 1;
}