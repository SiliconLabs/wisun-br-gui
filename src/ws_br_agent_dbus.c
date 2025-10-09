/***************************************************************************//**
 * @file ws_br_agent_dbus.c
 * @brief D-Bus interface for Wi-SUN SoC Border Router Agent
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <assert.h>
#include <systemd/sd-bus.h>

#include "ws_br_agent_dbus.h"
#include "ws_br_agent_log.h"
#include "ws_br_agent_utils.h"
#include "ws_br_agent_soc_host.h"

#define WS_BR_AGENT_DBUS_PATH "/com/silabs/Wisun/BorderRouter"
#define WS_BR_AGENT_DBUS_INTERFACE "com.silabs.Wisun.BorderRouter"
#define WS_BR_AGENT_DBUS_PROPERTY_ROUTING_GRAPH "RoutingGraph"
#define WS_BR_AGENT_DBUS_PROPERTY_NETWORK_NAME "WisunNetworkName"
#define WS_BR_AGENT_DBUS_PROPERTY_NETWORK_SIZE "WisunSize"
#define WS_BR_AGENT_DBUS_PROPERTY_REG_DOMAIN "WisunDomain"
#define WS_BR_AGENT_DBUS_PROPERTY_PHY_MODE_ID "WisunPhyModeId"
#define WS_BR_AGENT_DBUS_PROPERTY_CHAN_PLAN_ID "WisunChanPlanId"
#define WS_BR_AGENT_DBUS_PROPERTY_FAN_VERSION "WisunFanVersion"
#define WS_BR_AGENT_DBUS_METHOD_START_SOC_BORDER_ROUTER "RestartSoCBorderRouter"
#define WS_BR_AGENT_DBUS_METHOD_STOP_SOC_BORDER_ROUTER "StopSoCBorderRouter"
#define WS_BR_AGENT_DBUS_METHOD_SET_SOC_BORDER_ROUTER_CONFIG "SetSoCBorderRouterConfig"

static void dbus_thr_fnc(void *arg);
static int dbus_get_routing_graph(sd_bus *bus, const char *path, const char *interface,
                                  const char *property, sd_bus_message *reply, 
                                  void *userdata, sd_bus_error *ret_error);
static int dbus_get_network_name(sd_bus *bus, const char *path, const char *interface,
                                 const char *property, sd_bus_message *reply, 
                                 void *userdata, sd_bus_error *ret_error);
static int dbus_get_network_size(sd_bus *bus, const char *path, const char *interface,
                                 const char *property, sd_bus_message *reply, 
                                 void *userdata, sd_bus_error *ret_error);
static int dbus_get_reg_domain(sd_bus *bus, const char *path, const char *interface,
                               const char *property, sd_bus_message *reply, 
                               void *userdata, sd_bus_error *ret_error);
static int dbus_get_phy_mode_id(sd_bus *bus, const char *path, const char *interface,
                                const char *property, sd_bus_message *reply, 
                                void *userdata, sd_bus_error *ret_error);
static int dbus_get_chan_plan_id(sd_bus *bus, const char *path, const char *interface,
                                 const char *property, sd_bus_message *reply, 
                                 void *userdata, sd_bus_error *ret_error);
static int dbus_get_fan_version(sd_bus *bus, const char *path, const char *interface,
                               const char *property, sd_bus_message *reply, 
                               void *userdata, sd_bus_error *ret_error);

static int dbus_method_restart_br(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
static int dbus_method_stop_br(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
static int dbus_method_set_config(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);

static ws_br_agent_ret_t dbus_init(sd_bus **bus, sd_bus_slot **slot);static bool is_zero_addr(const uint8_t addr[16]);

static pthread_t dbus_thr;
static sd_bus *bus = NULL;
static sd_bus_slot *slot = NULL;
static volatile sig_atomic_t dbus_thread_stop = 0;

static const sd_bus_vtable dbus_vtable[] = {
  SD_BUS_VTABLE_START(0),
  SD_BUS_METHOD(WS_BR_AGENT_DBUS_METHOD_START_SOC_BORDER_ROUTER, "", NULL, 
                dbus_method_restart_br, 0),
  SD_BUS_METHOD(WS_BR_AGENT_DBUS_METHOD_STOP_SOC_BORDER_ROUTER, "", NULL, 
                dbus_method_stop_br, 0),
  SD_BUS_METHOD(WS_BR_AGENT_DBUS_METHOD_SET_SOC_BORDER_ROUTER_CONFIG, "", NULL, 
                dbus_method_set_config, 0),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_ROUTING_GRAPH, "a(aybaay)", 
                  dbus_get_routing_graph, 0, SD_BUS_VTABLE_PROPERTY_EMITS_INVALIDATION),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_NETWORK_NAME, "s", 
                  dbus_get_network_name, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_NETWORK_SIZE, "s", 
                  dbus_get_network_size, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_REG_DOMAIN, "s", 
                  dbus_get_reg_domain, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_PHY_MODE_ID, "u", 
                  dbus_get_phy_mode_id, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_CHAN_PLAN_ID, "u", 
                  dbus_get_chan_plan_id, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_FAN_VERSION, "y", 
                  dbus_get_fan_version, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  SD_BUS_VTABLE_END
};


ws_br_agent_ret_t ws_br_agent_dbus_init(void) 
{
  pthread_attr_t attr;
  size_t stack_size = 1024 * 1024; // 1MB stack size
  
  // Initialize thread attributes
  if (pthread_attr_init(&attr) != 0) {
    ws_br_agent_log_error("Failed to initialize thread attributes\n");
    return WS_BR_AGENT_RET_ERR;
  }
  
  // Set stack size
  if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
    ws_br_agent_log_error("Failed to set thread stack size\n");
    pthread_attr_destroy(&attr);
    return WS_BR_AGENT_RET_ERR;
  }
  
  // Create thread with increased stack size
  if (pthread_create(&dbus_thr, &attr, (void *)dbus_thr_fnc, NULL) != 0) {
    ws_br_agent_log_error("Failed to create D-Bus thread\n");
    pthread_attr_destroy(&attr);
    return WS_BR_AGENT_RET_ERR;
  }
  
  pthread_attr_destroy(&attr);
  return WS_BR_AGENT_RET_OK;
}

void ws_br_agent_dbus_deinit(void)
{
  dbus_thread_stop = 1;
  pthread_join(dbus_thr, NULL);
}

ws_br_agent_ret_t ws_br_agent_dbus_notify_topology_changed(void)
{
  if (bus == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }

  if (sd_bus_emit_properties_changed(bus, WS_BR_AGENT_DBUS_PATH, 
                                     WS_BR_AGENT_DBUS_INTERFACE, 
                                     WS_BR_AGENT_DBUS_PROPERTY_ROUTING_GRAPH, NULL) < 0) {
    return WS_BR_AGENT_RET_ERR;
  }

  return WS_BR_AGENT_RET_OK;
}

ws_br_agent_ret_t ws_br_agent_dbus_notify_settings_changed(void)
{
  if (bus == NULL) {
    return WS_BR_AGENT_RET_ERR;
  }
  // Notify D-Bus clients that the any of settings property has changed
  if (sd_bus_emit_properties_changed(bus, WS_BR_AGENT_DBUS_PATH, 
                                     WS_BR_AGENT_DBUS_INTERFACE, 
                                     WS_BR_AGENT_DBUS_PROPERTY_NETWORK_NAME, 
                                     WS_BR_AGENT_DBUS_PROPERTY_NETWORK_SIZE,
                                     WS_BR_AGENT_DBUS_PROPERTY_REG_DOMAIN,
                                     WS_BR_AGENT_DBUS_PROPERTY_PHY_MODE_ID,
                                     WS_BR_AGENT_DBUS_PROPERTY_CHAN_PLAN_ID,
                                     WS_BR_AGENT_DBUS_PROPERTY_FAN_VERSION,
                                     NULL) < 0) {
    return WS_BR_AGENT_RET_ERR;
  }
  
  return WS_BR_AGENT_RET_OK;
}

static ws_br_agent_ret_t dbus_init(sd_bus **bus, sd_bus_slot **slot)
{
  int r;
  r = sd_bus_open_system(bus);
  if (r < 0) {
    ws_br_agent_log_error("Failed to connect to system bus: %s\n", strerror(-r));
    return WS_BR_AGENT_RET_ERR;
  }

  r = sd_bus_add_object_vtable(*bus, slot,
                               WS_BR_AGENT_DBUS_PATH,
                               WS_BR_AGENT_DBUS_INTERFACE,
                               dbus_vtable, NULL);
  if (r < 0) {
    ws_br_agent_log_error("Failed to add vtable: %s\n", strerror(-r));
    return WS_BR_AGENT_RET_ERR;
  }

  r = sd_bus_request_name(*bus, "com.silabs.Wisun.BorderRouter", 0);
  if (r < 0) {
    ws_br_agent_log_error("Failed to acquire service name: %s\n", strerror(-r));
    return WS_BR_AGENT_RET_ERR;
  }

  return WS_BR_AGENT_RET_OK;
} 

// D-Bus property getter for RoutingGraph
static int dbus_get_routing_graph(sd_bus *bus, const char *path, const char *interface,
                                  const char *property, sd_bus_message *reply, 
                                  void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_soc_host_topology_t topology = {0U, NULL};
  int r = -1;

  if (ws_br_agent_soc_host_get_topology(&topology) != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("Failed to get topology for D-Bus property\n");
    return -1;
  }

  if (topology.entry_count == 0 || topology.entries == NULL) {
    return sd_bus_message_append(reply, "a(aybaay)", 0);
  }

  r = sd_bus_message_open_container(reply, 'a', "(aybaay)");
  if (r < 0) return r;

  for (size_t i = 0; i < topology.entry_count; ++i) {
    ws_br_agent_soc_host_topology_entry_t *entry = &topology.entries[i];
    r = sd_bus_message_open_container(reply, 'r', "aybaay");
    if (r < 0) return r;
    r = sd_bus_message_append_array(reply, 'y', entry->target, 16);
    if (r < 0) return r;
    r = sd_bus_message_append(reply, "b", false); // or true if external
    if (r < 0) return r;
    r = sd_bus_message_open_container(reply, 'a', "ay");
    if (r < 0) return r;
    // Only append if it's not BR (Always first element)
    if (i) {
      r = sd_bus_message_append_array(reply, 'y', entry->preferred, 16);
      if (r < 0) return r;
      if (!is_zero_addr(entry->backup)) {
        r = sd_bus_message_append_array(reply, 'y', entry->backup, 16);
        if (r < 0) return r;
      }
    } 
    r = sd_bus_message_close_container(reply); // close 'a'
    if (r < 0) return r;
    r = sd_bus_message_close_container(reply); // close 'r'
    if (r < 0) return r;
  }

  r = sd_bus_message_close_container(reply); // close 'a'

  (void) ws_br_agent_soc_host_free_topology(&topology);

  return r;
}


static int dbus_get_network_name(sd_bus *bus, const char *path, const char *interface,
                                 const char *property, sd_bus_message *reply, 
                                 void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_settings_t settings = { 0U };
  int r = -1;

  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    return -1;
  }
  
  r = sd_bus_message_append(reply, "s", settings.network_name);
  
  return r;
}

static int dbus_get_network_size(sd_bus *bus, const char *path, const char *interface,
                                 const char *property, sd_bus_message *reply, 
                                 void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_settings_t settings = { 0U };
  int r = -1;

  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    return -1;
  }
  
  r = sd_bus_message_append(reply, "s",
                            ws_br_agent_utils_val_to_str(settings.network_size, 
                                                         ws_br_agent_nw_size_strs, 
                                                         "Unknown"));
  
  return r;
}

static int dbus_get_reg_domain(sd_bus *bus, const char *path, const char *interface,
                               const char *property, sd_bus_message *reply, 
                               void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_settings_t settings = { 0U };
  int r = -1;
  uint8_t value = 0;

  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    return -1;
  }
  switch (settings.phy.type) {
    case WS_BR_AGENT_PHY_CONFIG_FAN11:
      value = settings.phy.config.fan11.reg_domain;
      break;
    
    case WS_BR_AGENT_PHY_CONFIG_FAN10:
      value = settings.phy.config.fan10.reg_domain;
      break;

    default: 
      value = 0U;
      break;
  }

  r = sd_bus_message_append(reply, "s",
                            ws_br_agent_utils_val_to_str(value, 
                                                         ws_br_agent_domains_strs, 
                                                         "Unknown"));

  return r;
}

static int dbus_get_phy_mode_id(sd_bus *bus, const char *path, const char *interface,
                               const char *property, sd_bus_message *reply, 
                               void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_settings_t settings = { 0U };
  int r = -1;
  uint32_t value = 0U;

  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    return -1;
  }
  value = settings.phy.type != WS_BR_AGENT_PHY_CONFIG_FAN11 ? 0U :
           settings.phy.config.fan11.phy_mode_id;
  r = sd_bus_message_append(reply, "u", value);

  return r;
}

static int dbus_get_chan_plan_id(sd_bus *bus, const char *path, const char *interface,
                                 const char *property, sd_bus_message *reply, 
                                 void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_settings_t settings = { 0U };
  int r = -1;
  uint32_t value = 0U;

  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    return -1;
  }
  value = settings.phy.type != WS_BR_AGENT_PHY_CONFIG_FAN11 ? 0U :
           settings.phy.config.fan11.chan_plan_id;

  r = sd_bus_message_append(reply, "u", value);

  return r;
}

static int dbus_get_fan_version(sd_bus *bus, const char *path, const char *interface,
                               const char *property, sd_bus_message *reply, 
                               void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_settings_t settings = { 0U };
  int r = -1;
  uint8_t value = 0U;

  if (ws_br_agent_soc_host_get_settings(&settings) != WS_BR_AGENT_RET_OK) {
    return -1;
  }

  switch (settings.phy.type) {
    case WS_BR_AGENT_PHY_CONFIG_FAN11:
      value = 2U;
      break;
    
    case WS_BR_AGENT_PHY_CONFIG_FAN10:
      value = 1U;
      break;

    default: 
      value = 0U;
      break;
  }

  r = sd_bus_message_append(reply, "y", value);

  return r;
}

static int dbus_method_restart_br(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_msg_t msg = { 
    .msg_code = WS_BR_AGENT_MSG_CODE_RESTART_BR, 
    .payload = NULL, 
    .payload_len = 0 
  };
  
  (void)userdata;
  
  ws_br_agent_log_info("D-Bus method RestartSoCBorderRouter called\n");
  // Send request to SoC host
  if (ws_br_agent_soc_host_send_req(&msg, NULL) != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("D-Bus: Failed to send restart BR request to SoC host\n");
  } else {
    ws_br_agent_log_info("D-Bus: Restart BR request sent successfully\n");
  }
  
  return sd_bus_reply_method_return(m, NULL);
}

static int dbus_method_stop_br(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_msg_t msg = { 
    .msg_code = WS_BR_AGENT_MSG_CODE_STOP_BR, 
    .payload = NULL, 
    .payload_len = 0 
  };
  
  (void)userdata;
  
  ws_br_agent_log_info("D-Bus method StopSoCBorderRouter called\n");
  // Send request to SoC host
  if (ws_br_agent_soc_host_send_req(&msg, NULL) != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("D-Bus: Failed to send stop BR request to SoC host\n");
  } else {
    ws_br_agent_log_info("D-Bus: Stop BR request sent successfully\n");
  }
  
  return sd_bus_reply_method_return(m, NULL);
}

static int dbus_method_set_config(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
  ws_br_agent_msg_t msg = {
    .msg_code = WS_BR_AGENT_MSG_CODE_SET_CONFIG_PARAMS,
    .payload = NULL,
    .payload_len = sizeof(ws_br_agent_settings_t)
  };
  
  (void)userdata;
  
  ws_br_agent_log_info("D-Bus method SetSoCBorderRouterConfig called\n");
  // Send request to SoC host
  if (ws_br_agent_soc_host_send_req(&msg, NULL) != WS_BR_AGENT_RET_OK) {
    ws_br_agent_log_error("D-Bus: Failed to send set config request to SoC host\n");
  } else {
    ws_br_agent_log_info("D-Bus: Set config request sent successfully\n");
  }
  
  return sd_bus_reply_method_return(m, NULL);
}

static void dbus_thr_fnc(void *arg)
{
  (void) arg;

  assert(dbus_init(&bus, &slot) == WS_BR_AGENT_RET_OK);
  ws_br_agent_log_warn("D-Bus service started\n");
  while (!dbus_thread_stop) {
    sd_bus_process(bus, NULL);
    if (dbus_thread_stop) {
      break;
    }
    sd_bus_wait(bus, 1000000UL); // 1 second
  }

  sd_bus_slot_unref(slot);
  sd_bus_unref(bus);
  ws_br_agent_log_warn("D-Bus service stopped\n");
}

static bool is_zero_addr(const uint8_t addr[16]) 
{
  for (size_t i = 0; i < 16U; ++i) {
    if (addr[i]) {
      return false;
    }
  }
  return true;
}