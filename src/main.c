#include <stdlib.h>
#include "ws_br_agent_defs.h"
#include "ws_br_agent_log.h"

int main(int argc, char *argv[]) {
    ws_br_agent_app_print("***********************************************************"                    "*\n");
    ws_br_agent_app_print("*          " WS_BR_AGENT_APP_NAME "                        "                    "*\n");
    ws_br_agent_app_print("*                        v" WS_BR_AGENT_VERSION "                              ""*\n");
    ws_br_agent_app_print("*           " WS_BR_AGENT_APP_COPYRIGHT_STR "              "                    "*\n");
    ws_br_agent_app_print("***********************************************************"                    "*\n");

    
    return EXIT_SUCCESS;
}
