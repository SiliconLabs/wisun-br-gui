#!/bin/bash

# Shell script to call the SetSoCBorderRouterConfig D-Bus method
# Usage: ./dbus-set-config.sh

dbus-send --system --print-reply \
    --dest=com.silabs.Wisun.SocBorderRouterAgent \
    /com/silabs/Wisun/SocBorderRouterAgent \
    com.silabs.Wisun.SocBorderRouterAgent.SetSoCBorderRouterConfig