#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_clnt.h"
#include "ws_br_agent_utils.h"

  // TODO Replace with settings API
  const ws_br_agent_param_payload_t ws_br_agent_default_settings = {
    .network_name = "Wi-SUN_Network",
    .regulatory_domain = 1,
    .network_size = 0,
    .chan_plan_id = 0,
    .phy_mode_id = 0,
    .tx_power_ddbm = 0,
    .pan_id = 0x1234,
    .gaks = { {0}, {0}, {0}, {0} },
    .gtks = { {0}, {0}, {0}, {0} }
  };

static const char *remote_addr_str = "::";

static struct sockaddr_in6 remote_addr = {
  .sin6_family = AF_INET6,
  .sin6_port = htons(WS_BR_AGENT_SOC_PORT),
  .sin6_addr = IN6ADDR_ANY_INIT
};

int32_t ws_br_agent_clnt_send_req(const ws_br_agent_msg_t * const req_msg, 
                                  ws_br_agent_clnt_process_resp_cb_t resp_cb)
{
  int sockfd;
  ssize_t r = 0;
  size_t buf_size = 0;
  uint8_t *rxtx_buf = NULL;
  ws_br_agent_msg_t *msg = NULL;

  ws_br_agent_log_info("Send '%s' request (0x%2x)...\n", 
                       ws_br_utils_get_req_code_str(req_msg->msg_code), req_msg->msg_code);

  sockfd = socket(AF_INET6, SOCK_STREAM, 0);

  if (sockfd < 0) {
    ws_br_agent_log_error("Failed: Socket creation\n");
    return WS_BR_AGENT_RET_ERR;
  }
  
  if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
    ws_br_agent_log_error("Failed: Connection to %s:%u\n", remote_addr_str, WS_BR_AGENT_SOC_PORT);
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  rxtx_buf = ws_br_agent_msg_build_buf(req_msg, &buf_size);
  if (rxtx_buf == NULL || buf_size < WS_BR_AGENT_MSG_MIN_BUF_SIZE) {
    ws_br_agent_log_error("Failed: Building request\n");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  if (send(sockfd, rxtx_buf, buf_size, 0) < 0) { 
    ws_br_agent_log_error("Failed: Sending request\n");
    free(rxtx_buf);
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }
  free(rxtx_buf);

  // Receive data
  rxtx_buf = malloc(WS_BR_AGENT_MAX_BUF_SIZE);
  if (rxtx_buf == NULL) {
    ws_br_agent_log_error("Failed: Memory allocation\n");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  r = recv(sockfd, rxtx_buf, WS_BR_AGENT_MAX_BUF_SIZE, 0);

  // No response or error
  if (!r) {
    close(sockfd);
    free(rxtx_buf);
    return WS_BR_AGENT_RET_OK;
  
  } else if (r < WS_BR_AGENT_MSG_MIN_BUF_SIZE) {
    ws_br_agent_log_error("Failed: Receiving response\n");
    free(rxtx_buf);
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Received response (%ld bytes)\n", r);
  msg = ws_br_agent_msg_parse_buf(rxtx_buf, (size_t)r);
  free(rxtx_buf);

  if (msg == NULL) {
    ws_br_agent_log_error("Failed: Parsing response\n");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  if (resp_cb != NULL) {
    if (resp_cb(msg) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_warn("Response process callback failed\n");
      ws_br_agent_msg_free(msg);
      close(sockfd);
      return WS_BR_AGENT_RET_ERR;
    }
  }
  ws_br_agent_msg_free(msg);
  close(sockfd);
  ws_br_agent_log_info("OK\n");
  
  return WS_BR_AGENT_RET_OK;
}

int32_t ws_br_agent_clnt_set_remote_addr(const char *addr)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  if (inet_pton(AF_INET6, addr, &remote_addr.sin6_addr) != 1) {
    ws_br_agent_log_error("Invalid IPv6 address: %s", addr);
    return WS_BR_AGENT_RET_ERR;
  }

  remote_addr_str = addr;
  return WS_BR_AGENT_RET_OK;
}



