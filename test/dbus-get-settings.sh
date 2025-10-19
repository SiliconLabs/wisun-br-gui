#!/bin/bash
# Get Wi-SUN settings from D-Bus and print values

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

echo "PAN ID:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.BorderRouter" string:"WisunPanId"