#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ws_br_agent_log.h"
#include "ws_br_agent_defs.h"
#include "ws_br_agent_msg.h"
#include "ws_br_agent_soc_host.h"
#include "ws_br_agent_utils.h"

#include <pthread.h>


static ws_br_agent_ret_t copy_topology(ws_br_agent_soc_host_topology_t * const dst_topology,
                                       const ws_br_agent_soc_host_topology_t * const src_topology);

static const ws_br_agent_settings_t default_host_settings = {
  .network_name = "Wi-SUN Network",
  .regulatory_domain = 1,
  .network_size = 0,
  .chan_plan_id = 0,
  .phy_mode_id = 0,
  .tx_power_ddbm = 0,
  .pan_id = 0x1234,
  .gaks = { {0}, {0}, {0}, {0} },
  .gtks = {
    {0xBB, 0x06, 0x08, 0x57, 0x2C, 0xE1, 0x4D, 0x7B, 0xA2, 0xD1, 0x55, 0x49, 0x9C, 0xC8, 0x51, 0x9B},  // GTK[0] from Conformance Test Plan.
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  }
};

static ws_br_agent_soc_host_t host = { 0U };

static pthread_mutex_t host_mutex = PTHREAD_MUTEX_INITIALIZER;

static ws_br_agent_soc_host_topology_t host_topology = { 
  .entry_count = 0U, 
  .entries = NULL 
};

ws_br_agent_ret_t ws_br_agent_soc_host_init(void) 
{
  if (pthread_mutex_init(&host_mutex, NULL) != 0) {
    ws_br_agent_log_error("Mutex init failed\n");
    return WS_BR_AGENT_RET_ERR;
  }
  // Set local host with default settings for init
  ws_br_agent_soc_host_set("::1", NULL);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_send_req(const ws_br_agent_msg_t * const req_msg, 
                                                ws_br_agent_soc_host_process_resp_cb_t resp_cb)
{
  int sockfd = -1;
  ssize_t r = 0;
  size_t buf_size = 0;
  uint8_t *rxtx_buf = NULL;
  ws_br_agent_msg_t *msg = NULL;

  pthread_mutex_lock(&host_mutex);
  ws_br_agent_log_info("Send '%s' request (0x%08x)...\n", 
                       ws_br_agent_utils_get_req_code_str(req_msg->msg_code), req_msg->msg_code);

  sockfd = socket(AF_INET6, SOCK_STREAM, 0);

  if (sockfd < 0) {
    ws_br_agent_log_error("Failed: Socket creation\n");
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  if (connect(sockfd, (struct sockaddr *)&host.remote_addr, sizeof(host.remote_addr)) < 0) {
    ws_br_agent_log_error("Failed: Connection to %s:%u\n", host.remote_addr_str, WS_BR_AGENT_SOC_PORT);
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  rxtx_buf = ws_br_agent_msg_build_buf(req_msg, &buf_size);
  if (rxtx_buf == NULL || buf_size < WS_BR_AGENT_MSG_MIN_BUF_SIZE) {
    ws_br_agent_log_error("Failed: Building request\n");
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  if (send(sockfd, rxtx_buf, buf_size, 0) < 0) { 
    ws_br_agent_log_error("Failed: Sending request\n");
    free(rxtx_buf);
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }
  free(rxtx_buf);

  // Receive data
  rxtx_buf = malloc(WS_BR_AGENT_MAX_BUF_SIZE);
  if (rxtx_buf == NULL) {
    ws_br_agent_log_error("Failed: Memory allocation\n");
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  r = recv(sockfd, rxtx_buf, WS_BR_AGENT_MAX_BUF_SIZE, 0);

  // No response or error
  if (!r) {
    close(sockfd);
    free(rxtx_buf);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_OK;
  
  } else if (r < WS_BR_AGENT_MSG_MIN_BUF_SIZE) {
    ws_br_agent_log_error("Failed: Receiving response\n");
    free(rxtx_buf);
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  ws_br_agent_log_info("Received response (%ld bytes)\n", r);
  msg = ws_br_agent_msg_parse_buf(rxtx_buf, (size_t)r);
  free(rxtx_buf);

  if (msg == NULL) {
    ws_br_agent_log_error("Failed: Parsing response\n");
    close(sockfd);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  if (resp_cb != NULL) {
    if (resp_cb(msg) != WS_BR_AGENT_RET_OK) {
      ws_br_agent_log_warn("Response process callback failed\n");
      ws_br_agent_msg_free(msg);
      close(sockfd);
      pthread_mutex_unlock(&host_mutex);
      return WS_BR_AGENT_RET_ERR;
    }
  }
  ws_br_agent_msg_free(msg);
  close(sockfd);
  ws_br_agent_log_info("OK\n");
  pthread_mutex_unlock(&host_mutex);
  
  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set(const char *addr,
                                           const ws_br_agent_settings_t *const settings)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  pthread_mutex_lock(&host_mutex);
  if (inet_pton(AF_INET6, addr, &host.remote_addr.sin6_addr) != 1) {
    ws_br_agent_log_error("Invalid IPv6 address: %s\n", addr);
    pthread_mutex_unlock(&host_mutex);
    return WS_BR_AGENT_RET_ERR;
  }

  snprintf(host.remote_addr_str, sizeof(host.remote_addr_str), "%s", addr);
  host.remote_addr_str[WS_BR_AGENT_IPV6_ADDR_STR_SIZE - 1] = '\0';

  if (settings != NULL) {
    memcpy(&host.settings, settings, sizeof(ws_br_agent_settings_t));
  } else {
    memcpy(&host.settings, &default_host_settings, sizeof(ws_br_agent_settings_t));
  }
  pthread_mutex_unlock(&host_mutex);
  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_get(ws_br_agent_soc_host_t * const dst_host)
{
  if (dst_host == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(dst_host, &host, sizeof(ws_br_agent_soc_host_t));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set_remote_addr(const struct sockaddr_in6 * const addr)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(&host.remote_addr, addr, sizeof(struct sockaddr_in6));
  host.remote_addr.sin6_port = htons(WS_BR_AGENT_SOC_PORT);
  inet_ntop(AF_INET6, &addr->sin6_addr, host.remote_addr_str, sizeof(host.remote_addr_str));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}
ws_br_agent_ret_t ws_br_agent_soc_host_get_remote_addr(struct sockaddr_in6 * const addr)
{
  if (addr == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(addr, &host.remote_addr, sizeof(struct sockaddr_in6));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

const ws_br_agent_settings_t *ws_br_agent_soc_host_get_default_settings(void)
{
  const ws_br_agent_settings_t *ret = &default_host_settings;
  return ret;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set_settings(const ws_br_agent_settings_t * const settings)
{
  if (settings == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(&host.settings, settings, sizeof(ws_br_agent_settings_t));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_get_settings(ws_br_agent_settings_t * const settings)
{
  if (settings == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  pthread_mutex_lock(&host_mutex);
  memcpy(settings, &host.settings, sizeof(ws_br_agent_settings_t));
  pthread_mutex_unlock(&host_mutex);

  return WS_BR_AGENT_RET_OK;
}

static ws_br_agent_ret_t copy_topology(ws_br_agent_soc_host_topology_t * const dst_topology,
                                       const ws_br_agent_soc_host_topology_t * const src_topology)
{
  size_t storage_size = 0U;

  if (src_topology == NULL
     || dst_topology == NULL
     || src_topology->entries == NULL 
     || !src_topology->entry_count) {
    return WS_BR_AGENT_RET_ERR;
  }

  if (dst_topology->entries != NULL) {
    free(dst_topology->entries);
  }

  dst_topology->entry_count = src_topology->entry_count;

  // Allocate dest topology
  storage_size = src_topology->entry_count * sizeof(ws_br_agent_soc_host_topology_entry_t);
  dst_topology->entries = (ws_br_agent_soc_host_topology_entry_t *) malloc(storage_size);
  if (dst_topology->entries == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  memcpy(dst_topology->entries, src_topology->entries, storage_size);

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_soc_host_set_topology(const ws_br_agent_soc_host_topology_t *topology)
{
  ws_br_agent_ret_t ret = WS_BR_AGENT_RET_ERR;

  pthread_mutex_lock(&host_mutex);
  ret = copy_topology(&host_topology, topology);
  pthread_mutex_unlock(&host_mutex);

  return ret;
}

ws_br_agent_ret_t ws_br_agent_soc_host_get_topology(ws_br_agent_soc_host_topology_t * const topology)
{
  ws_br_agent_ret_t ret = WS_BR_AGENT_RET_ERR;

  pthread_mutex_lock(&host_mutex);
  ret = copy_topology(topology, &host_topology);
  pthread_mutex_unlock(&host_mutex);

  return ret;
}

ws_br_agent_ret_t ws_br_agent_soc_host_free_topology(ws_br_agent_soc_host_topology_t *topology)
{
  if (topology == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  if (topology->entries != NULL) {
    free(topology->entries);
    topology->entries = NULL;
  }

  topology->entry_count = 0;

  return WS_BR_AGENT_RET_OK;
}