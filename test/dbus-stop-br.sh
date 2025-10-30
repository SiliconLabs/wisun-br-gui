#!/bin/bash

# Shell script to call the StopSoCBorderRouter D-Bus method
# Usage: ./dbus-stop-br.sh

dbus-send --system --print-reply \
    --dest=com.silabs.Wisun.SocBorderRouterAgent \
    /com/silabs/Wisun/SocBorderRouterAgent \
    com.silabs.Wisun.SocBorderRouterAgent.StopSoCBorderRouter