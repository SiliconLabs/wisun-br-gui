#!/bin/bash
# Get Wi-SUN settings from D-Bus and print values

# Network name:
# method return time=1758900850.351627 sender=:1.208 -> destination=:1.211 serial=14 reply_serial=2
#    variant       string "bed3"
# Network size:
# method return time=1758900850.369295 sender=:1.208 -> destination=:1.212 serial=15 reply_serial=2
#    variant       string "SMALL"
# Regulatory domain:
# method return time=1758900850.384999 sender=:1.208 -> destination=:1.213 serial=16 reply_serial=2
#    variant       string "EU"
# Phy mode ID:
# method return time=1758900850.400877 sender=:1.208 -> destination=:1.214 serial=17 reply_serial=2
#    variant       uint32 1
# Channel plan ID:
# method return time=1758900850.416501 sender=:1.208 -> destination=:1.215 serial=18 reply_serial=2
#    variant       uint32 32
# FAN version:
# method return time=1758900850.432236 sender=:1.208 -> destination=:1.216 serial=19 reply_serial=2
#    variant       byte 2

echo "Network name:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunNetworkName"

echo "Network size:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunSize"

echo "Regulatory domain:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunDomain"

echo "Phy mode ID:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunPhyModeId"

echo "Channel plan ID:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunChanPlanId"

echo "FAN version:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunFanVersion"