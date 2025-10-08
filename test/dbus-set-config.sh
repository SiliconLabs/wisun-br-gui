#!/bin/bash

# Shell script to call the SetSoCBorderRouterConfig D-Bus method
# Usage: ./dbus-set-config.sh

dbus-send --system --print-reply \
    --dest=com.silabs.Wisun.BorderRouter \
    /com/silabs/Wisun/BorderRouter \
    com.silabs.Wisun.BorderRouter.SetSoCBorderRouterConfig