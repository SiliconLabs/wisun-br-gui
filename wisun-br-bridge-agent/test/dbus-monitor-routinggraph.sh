#!/bin/bash
# test/dbus-monitor-routinggraph.sh
# Monitors D-Bus for RoutingGraph property changes

busctl monitor com.silabs.Wisun.SocBorderRouterAgent | grep PropertiesChanged
