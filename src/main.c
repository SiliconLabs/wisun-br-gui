#include <assert.h>
#include <stdlib.h>
#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_clnt.h"

static void print_app_banner(void);

int main(int argc, char *argv[]) {
    
    print_app_banner();

    assert(ws_br_agent_clnt_init() == WS_BR_AGENT_RET_OK);

    pthread_join(clnt_thr, NULL);

    return EXIT_SUCCESS;
}

static void print_app_banner(void)
{
  const int banner_width = 60;
  int len_app_name = sizeof(WS_BR_AGENT_APP_NAME) - 1;
  int len_version = sizeof(WS_BR_AGENT_VERSION) - 1;
  int len_copyright = sizeof(WS_BR_AGENT_APP_COPYRIGHT_STR) - 1;
  int padding_left = 0;
  int padding_right = 0;

  for (int i = 0; i < banner_width; i++) ws_br_agent_app_print("*");
  ws_br_agent_app_print("\n");
  padding_left = (banner_width - 2 - len_app_name) / 2;
  padding_right = banner_width - 2 - len_app_name - padding_left;
  ws_br_agent_app_print("*%*c" WS_BR_AGENT_APP_NAME "%*c*\n", padding_left, ' ', padding_right, ' ');
  padding_left = (banner_width - 3 - len_version) / 2;
  padding_right = banner_width - 3 - len_version - padding_left;
  ws_br_agent_app_print("*%*cv" WS_BR_AGENT_VERSION "%*c*\n", padding_left, ' ', padding_right, ' ');
  padding_left = (banner_width - 2 - len_copyright) / 2;
  padding_right = banner_width - 2 - len_copyright - padding_left;
  ws_br_agent_app_print("*%*c" WS_BR_AGENT_APP_COPYRIGHT_STR "%*c*\n", padding_left, ' ', padding_right, ' ');
  for (int i = 0; i < banner_width; i++) ws_br_agent_app_print("*");
  ws_br_agent_app_print("\n");
}