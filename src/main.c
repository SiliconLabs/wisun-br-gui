#include <stdio.h>
#include "ws_br_agent_log.h"

int main(int argc, char *argv[]) {
    ws_br_agent_log_print(WS_BR_AGENT_LOG_COLOR_BLUE,"**********************************************************\n");
    ws_br_agent_log_print(WS_BR_AGENT_LOG_COLOR_BLUE,"*          Wi-SUN SoC Border Router Agent Service        *\n");
    ws_br_agent_log_print(WS_BR_AGENT_LOG_COLOR_BLUE,"*                        v0.1                            *\n");
    ws_br_agent_log_print(WS_BR_AGENT_LOG_COLOR_BLUE,"*            Copyright (c) 2025 Silicon Labs, Inc.       *\n");
    ws_br_agent_log_print(WS_BR_AGENT_LOG_COLOR_BLUE,"**********************************************************\n");

    return 0;
}
