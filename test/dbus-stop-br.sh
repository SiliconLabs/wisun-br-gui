#!/bin/bash

# Shell script to call the StopSoCBorderRouter D-Bus method
# Usage: ./dbus-stop-br.sh

dbus-send --system --print-reply \
    --dest=com.silabs.Wisun.BorderRouter \
    /com/silabs/Wisun/BorderRouter \
    com.silabs.Wisun.BorderRouter.StopSoCBorderRouter