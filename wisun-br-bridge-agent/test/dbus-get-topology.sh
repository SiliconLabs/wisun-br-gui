#!/usr/bin/env bash

dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"RoutingGraph"