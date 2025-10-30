#!/bin/bash
# Get Wi-SUN settings from D-Bus and print values

echo "Network name:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunNetworkName"

echo "Network size:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunSize"

echo "Regulatory domain:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunDomain"

echo "Phy mode ID:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunPhyModeId"

echo "Channel plan ID:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunChanPlanId"

echo "FAN version:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunFanVersion"

echo "PAN ID:"
dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get string:"com.silabs.Wisun.SocBorderRouterAgent" string:"WisunPanId"