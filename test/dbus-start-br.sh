#!/bin/bash

# Shell script to call the StartSoCBorderRouter D-Bus method
# Usage: ./dbus-start-br.sh

dbus-send --system --print-reply \
    --dest=com.silabs.Wisun.BorderRouter \
    /com/silabs/Wisun/BorderRouter \
    com.silabs.Wisun.BorderRouter.StartSoCBorderRouter