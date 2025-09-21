# Wi-SUN SoC Border Router Agent

## Overview

This application implements a Wi-SUN SoC Border Router Agent, designed to manage and control a Wi-SUN Border Router device. It provides a network service interface for configuration, monitoring, and management tasks.

## Integration

The agent is intended to be managed via the [wisun-br-gui](https://github.com/SiliconLabs/wisun-br-gui) (Cockpit UI), which provides a graphical interface for users to interact with the Border Router, view network status, and perform configuration operations.

## Features

- TCP server for remote management
- Supports Wi-SUN network configuration and status queries
- Compatible with Silicon Labs Wi-SUN SoC platforms
- Designed for integration with cockpit UI

## Getting Started

1. Build the application using CMake and GCC on Linux.
2. Run the agent on your Wi-SUN SoC Border Router device.
3. Connect and manage the agent using the wisun-br-gui Cockpit UI.

## Repository Structure

- `src/` — Source code for the agent
- `inc/` — Header files
- `test/` — Test scripts and utilities
- `build/` — Build output (ignored in git)

## License

Copyright (c) 2025 Silicon Labs, Inc.

