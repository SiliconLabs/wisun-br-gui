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

static const char *remote_addr_str = NULL;

static struct sockaddr_in6 remote_addr = {
  .sin6_family = AF_INET6,
  .sin6_port = htons(WS_BR_AGENT_SOC_PORT),
  .sin6_addr = IN6ADDR_ANY_INIT
};

int32_t ws_br_agent_clnt_send_req(const uint32_t req_code, ws_br_agent_clnt_process_resp_cb_t resp_cb);
{
  int sockfd;
  ssize_t r = 0;
  static uint8_t buf[WS_BE_AGENT_MAX_BUF_SIZE] = { 0};

  ws_br_agent_log_info("Send '%s' request (0x%2x)...", 
                       ws_br_utils_get_req_code_str(req_code), req_code);

  sockfd = socket(AF_INET6, SOCK_STREAM, 0);

  if (sockfd < 0) {
    ws_br_agent_log_err("Socket creation failed");
    return WS_BR_AGENT_RET_ERR;
  }
  
  if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
    ws_br_agent_log_err("Connection to %s:%u failed", remote_addr_str ? remote_addr_str : "::", WS_BR_AGENT_SOC_PORT);
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }
  // build_req_payload(req_code, buf, &buf_size);

  ws_br_agent_msg_build_buf(req_code, buf, WS_BE_AGENT_MAX_BUF_SIZE);
  if (send(sockfd, buf, buf_size, 0) < 0) { 
    ws_br_agent_log_err("Sending request failed");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  // Receive data
  r = recv(sockfd, buf, WS_BE_AGENT_MAX_BUF_SIZE, 0);

  // No response or error
  if (!r) {
    close(sockfd);
    return WS_BR_AGENT_RET_OK;
  } else if (r < WS_BR_AGENT_MSG_DEFAULT_BUF_SIZE) {
    ws_br_agent_log_err("Receiving response failed");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Received response (%lu bytes)", r);

  if (resp_cb != NULL) {
    if (resp_cb(buf, (size_t)r) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_err("Response callback failed");
      close(sockfd);
      return WS_BR_AGENT_RET_ERR;
    }
  }
 
  close(sockfd);
  return WS_BR_AGENT_RET_OK;
}

int32_t ws_br_agent_clnt_set_remote_addr(const char *addr)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  if (inet_pton(AF_INET6, addr, &remote_addr.sin6_addr) != 1) {
    ws_br_agent_log_err("Invalid IPv6 address: %s", addr);
    return WS_BR_AGENT_RET_ERR;
  }

  remote_addr_str = addr;
  return WS_BR_AGENT_RET_OK;
}



