#!/usr/bin/env bash

dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"RoutingGraph"