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
#include "ws_br_agent_utils.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_srv.h"

#define DISPACH_DELAY_US 1000UL

pthread_t srv_thr;
volatile sig_atomic_t thread_stop;
static int listen_fd = -1L;
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
  
  
  int conn_fd = -1L;
  struct sockaddr_in6 serv_addr = { 0U }; 
  struct sockaddr_in6 client_addr = { 0U };
  socklen_t client_len = sizeof(client_addr);
  char client_ip[INET6_ADDRSTRLEN] = { 0U };
  static uint8_t buf[WS_BR_AGENT_MAX_BUF_SIZE] = { 0U };
  ssize_t r = 0L;
  ws_br_agent_msg_t *msg = NULL;

  ws_br_agent_log_warn("Server thread started");
  listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    ws_br_agent_log_error("Server socket creation failed");
    return;
  }

  serv_addr.sin6_family = AF_INET6;
  serv_addr.sin6_addr = in6addr_any;
  serv_addr.sin6_port = htons(WS_BR_AGENT_SERVICE_PORT);

  if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    ws_br_agent_log_error("Server bind failed");
    close(listen_fd);
    return;
  }

  if (listen(listen_fd, 5) < 0) {
    ws_br_agent_log_error("Server listen failed");
    close(listen_fd);
    return;
  }

  ws_br_agent_log_info("Server listening on port %d", WS_BR_AGENT_SERVICE_PORT);

  while (!thread_stop) {
    conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (conn_fd < 0) {
      if (thread_stop){
        break;
      }
      ws_br_agent_log_warn("Accept failed");
      usleep(DISPACH_DELAY_US);
      continue;
    }
    inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip, sizeof(client_ip));
    ws_br_agent_log_info("Accepted connection from %s:%d", client_ip, ntohs(client_addr.sin6_port));

    r = recv(conn_fd, buf, WS_BR_AGENT_MAX_BUF_SIZE, 0);
    if (r < 0) {
      ws_br_agent_log_warn("Receive failed");
      close(conn_fd);
      continue;
    } else if (r == 0) {
      ws_br_agent_log_warn("Connection closed by client");
      close(conn_fd);
      continue;
    }

    msg = ws_br_agent_msg_parse_buf(buf, (size_t)r);
    if (msg == NULL) {
      ws_br_agent_log_warn("Failed to parse received message");
      close(conn_fd);
      continue;
    }

    ws_br_agent_utils_print_msg(msg);
    
    ws_br_agent_msg_free(msg);
    
    close(conn_fd);
  }
  close(listen_fd);
  ws_br_agent_log_warn("Server thread stopped");
}

static void sigint_hnd(int signum) 
{
  (void) signum;
  thread_stop = 1;
  close(listen_fd);
  ws_br_agent_log_warn("Stop application...");
}