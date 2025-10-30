#!/bin/bash

# Shell script to call the RestartSoCBorderRouter D-Bus method
# Usage: ./dbus-restart-br.sh

dbus-send --system --print-reply \
    --dest=com.silabs.Wisun.SocBorderRouterAgent \
    /com/silabs/Wisun/SocBorderRouterAgent \
    com.silabs.Wisun.SocBorderRouterAgent.RestartSoCBorderRouter