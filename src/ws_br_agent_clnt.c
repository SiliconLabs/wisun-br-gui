#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_clnt.h"

#define REQ_RESP_BUF_SIZE 2048U
#define DISPACH_DELAY_US 1000UL
#define DEFAULT_MSG_BUF_SIZE (2 * sizeof(uint32_t))
#define SET_PARAM_MSG_BUF_SIZE (DEFAULT_MSG_BUF_SIZE + sizeof(ws_br_agent_param_payload_t))


#define __add_msg_code_and_len_to_buf(ptr, code, len) \
  do {                                                \
    *((uint32_t *)ptr) = htonl((uint32_t)code);       \
    ptr += sizeof(uint32_t);                          \
    *((uint32_t *)ptr) = htonl((uint32_t)len);        \
    ptr += sizeof(uint32_t);                          \
  } while(0)

pthread_t clnt_thr;

extern volatile sig_atomic_t thread_stop;

static const char *remote_addr_str = NULL;

static struct sockaddr_in6 remote_addr {
  .sin6_family = AF_INET6,
  .sin6_port = htons(WS_BR_AGENT_SOC_PORT),
  .sin6_addr = IN6ADDR_ANY_INIT
};

static ws_br_agent_param_payload_t params = { 0 };

static inline const char *get_req_code_str(const uint8_t req_code);

static void clnt_thr_fnc(void *arg);

static void build_req_payload(const uint8_t req_code, uint8_t *buf, size_t *buf_size);

static void print_msg(const uint8_t *buf, size_t buf_size);

int32_t ws_br_agent_clnt_init(void)
{
  if (pthread_create(&clnt_thr, NULL, (void *)clnt_thr_fnc, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }
  return WS_BR_AGENT_RET_OK;
}


int32_t ws_br_agent_clnt_send_req(const uint8_t req_code, ws_br_agent_clnt_resp_cb_t resp_cb)
{
  int sockfd;
  ssize_t r = 0;
  static uint8_t buf[REQ_RESP_BUF_SIZE] = { 0};
  size_t buf_size = REQ_RESP_BUF_SIZE;

  ws_br_agent_log_info("Send '%s' request (0x%2x)...", 
                       get_req_code_str(req_code), req_code);

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
  build_req_payload(req_code, buf, &buf_size);
  if (send(sockfd, buf, buf_size, 0) < 0) { 
    ws_br_agent_log_err("Sending request failed");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  // Receive data
  r = recv(sockfd, buf, REQ_RESP_BUF_SIZE, 0);

  // No response or error
  if (!r) {
    close(sockfd);
    return WS_BR_AGENT_RET_OK;
  } else if (r < 0) {
    ws_br_agent_log_err("Receiving response failed");
    close(sockfd);
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Received response (%lu bytes)", r);

  if (resp_cb != NULL) {
    if (resp_cb(recv_buf, (size_t)r) != WS_BR_AGENT_RET_OK) {
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

static void clnt_thr_fnc(void *arg)
{
  (void) arg;
  ws_br_agent_log_warn("Client thread started");

  while (!thread_stop) {
    usleep(DISPACH_DELAY_US);
  }
}

static inline const char *get_req_code_str(const uint8_t req_code)
{
  switch(req_code) {
    case WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY:
      return "GET_TOPOLOGY";
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
      return "GET_CONFIG_PARAMS";
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      return "SET_CONFIG_PARAMS";
    case WS_BR_AGENT_MSG_CODE_START_BR:
      return "START_BR";
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
      return "STOP_BR";
    default:
      return "UNKNOWN";
  }
}

static void print_msg(const uint8_t *buf, size_t buf_size)
{
  uint8_t *ptr = (uint8_t *)buf;
  if (buf == NULL || buf_size < DEFAULT_MSG_BUF_SIZE) {
    return;
  }

  ws_br_agent_log_info("Msg code: %04x\n", ntohl(*((uint32_t *)ptr)));
  ptr += sizeof(uint32_t);
  ws_br_agent_log_info("Payload len: %u\n", ntohl(*((uint32_t *)ptr)));
  ptr += sizeof(uint32_t);

  if (buf_size == DEFAULT_MSG_BUF_SIZE) {
    return;
  }

  ws_br_agent_log_info("Payload data:");
  for (size_t i = 0; i < (buf_size - DEFAULT_MSG_BUF_SIZE); i++) {
    if (i % 16 == 0) {
      ws_br_agent_log_info("\n%04x: ", (unsigned int)i);  
    }
    ws_br_agent_log_info("%02x ", buf[DEFAULT_MSG_BUF_SIZE + i]);
  }
  ws_br_agent_log_info("\n");
}


static int32_t build_req_payload(const uint8_t req_code, uint8_t *buf, size_t *buf_size)
{
  // Packet structure:
  // [msg code 4 byte] [payload len 4 byte] [payload data n byte]
  uint8_t *ptr = buf;

  if (buf == NULL || buf_size == NULL 
      || *buf_size < (2 * sizeof(uint32_t))) {
    return WS_BR_AGENT_RET_ERR;
  }

  switch(req_code) {
    case WS_BR_AGENT_MSG_CODE_GET_TOPOLOGY:
    case WS_BR_AGENT_MSG_CODE_GET_CONFIG_PARAMS:
    case WS_BR_AGENT_MSG_CODE_START_BR:
    case WS_BR_AGENT_MSG_CODE_STOP_BR:
      __add_msg_code_and_len_to_buf(ptr, req_code, 0);
      break;
    case WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS:
      __add_msg_code_and_len_to_buf(ptr, req_code, 
                                    sizeof(ws_br_agent_param_payload_t));
      memcpy(ptr, &param_payload, sizeof(ws_br_agent_param_payload_t));
      ptr += sizeof(ws_br_agent_param_payload_t);
    default:
      ws_br_agent_log_err("Build message error: Unsupported request code (0x%2x)", req_code);
      return WS_BR_AGENT_RET_ERRR;
  }
    
  *buf_size = (size_t)(ptr - buf);
  return WS_BR_AGENT_RET_OK;
}