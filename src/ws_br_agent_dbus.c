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
#include "ws_br_agent_soc_host.h"

#define WS_BR_AGENT_DBUS_PATH "/com/silabs/Wisun/BorderRouter"
#define WS_BR_AGENT_DBUS_INTERFACE "com.silabs.Wisun.BorderRouter"
#define WS_BR_AGENT_DBUS_PROPERTY_ROUTING_GRAPH "RoutingGraph"

static void dbus_thr_fnc(void *arg);
static int dbus_get_routing_graph(sd_bus *bus, const char *path, const char *interface,
                                  const char *property, sd_bus_message *reply, 
                                  void *userdata, sd_bus_error *ret_error);
static ws_br_agent_ret_t dbus_init(sd_bus **bus, sd_bus_slot **slot);

static pthread_t dbus_thr;
static sd_bus *bus = NULL;
static sd_bus_slot *slot = NULL;
static volatile sig_atomic_t dbus_thread_stop = 0;

// Vtable for the D-Bus interface
static const sd_bus_vtable dbus_vtable[] = {
  SD_BUS_VTABLE_START(0),
  SD_BUS_PROPERTY(WS_BR_AGENT_DBUS_PROPERTY_ROUTING_GRAPH, "a(aybaay)", 
                  dbus_get_routing_graph, 0, SD_BUS_VTABLE_PROPERTY_EMITS_INVALIDATION),
  SD_BUS_VTABLE_END
};


ws_br_agent_ret_t ws_br_agent_dbus_init(void) 
{
  // Init thread to call dbus process
  if (pthread_create(&dbus_thr, NULL, (void *)dbus_thr_fnc, NULL) != 0) {
    return WS_BR_AGENT_RET_ERR;
  }

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
  // Notify D-Bus clients that the RoutingGraph property has changed
  if (sd_bus_emit_properties_changed(bus, WS_BR_AGENT_DBUS_PATH, 
                                     WS_BR_AGENT_DBUS_INTERFACE, 
                                     WS_BR_AGENT_DBUS_PROPERTY_ROUTING_GRAPH, NULL) < 0) {
    ws_br_agent_log_error("Failed to emit properties changed signal\n");
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
    r = sd_bus_message_append_array(reply, 'y', entry->preferred, 16);
    if (r < 0) return r;
    r = sd_bus_message_append_array(reply, 'y', entry->backup, 16);
    if (r < 0) return r;
    r = sd_bus_message_close_container(reply); // close 'a'
    if (r < 0) return r;
    r = sd_bus_message_close_container(reply); // close 'r'
    if (r < 0) return r;
  }

  r = sd_bus_message_close_container(reply); // close 'a'

  (void) ws_br_agent_soc_host_free_topology(&topology);

  return r;
}

static void dbus_thr_fnc(void *arg)
{
  (void) arg;

  assert(dbus_init(&bus, &slot) == WS_BR_AGENT_RET_OK);
  ws_br_agent_log_warn("D-Bus service started\n");

  while (!dbus_thread_stop) {
    sd_bus_process(bus, NULL);
    sd_bus_wait(bus, 1000000UL); // 1 second
  }

  sd_bus_slot_unref(slot);
  sd_bus_unref(bus);
  ws_br_agent_log_warn("D-Bus service stopped\n");
}