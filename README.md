
# Wi-SUN SoC Border Router Agent

## Overview

Wi-SUN SoC Border Router Agent is a Linux-based service for managing and monitoring a Silicon Labs Wi-SUN Border Router. It exposes a TCP server for remote configuration and status queries, and integrates with D-Bus for IPC and system integration.

### Purpose

- Provide a remote management interface for Wi-SUN Border Routers.
- Enable network configuration, topology monitoring, and integration with UI tools (e.g., wisun-br-gui).
- Support automated and scriptable testing via Python and Linux utilities.

## Features

- TCP server for remote management and configuration
- D-Bus IPC interface for system integration and property signaling
- Thread-safe host and topology management
- Structured message protocol for configuration and topology
- Integration with Silicon Labs Wi-SUN SoC platforms
- Designed for use with graphical UI (wisun-br-gui) and automated scripts

## Build Steps

1. Install dependencies: GCC, CMake, systemd development headers (`libsystemd-dev`), pthreads.
2. Build the agent:
	 ```bash
	 mkdir -p build
	 cd build
	 cmake ..
	 make
	 ```
3. The binary will be located in `build/wisun-soc-br-agent`.

## Testing

### 1. Python Bot Test

- Use `test/br_soc_bot.py` to simulate TCP client requests and topology updates.
- Example:
	```bash
	python3 test/br_soc_bot.py
	```

### 2. Query RoutingGraph via D-Bus

- Use the provided shell script to query the RoutingGraph property:
	```bash
	bash test/dbus-get-topology.sh
	```

### 3. Monitor D-Bus Property Signaling

- Use the provided script to monitor D-Bus for RoutingGraph property changes:
	```bash
	bash test/dbus-monitor-routinggraph.sh
	```
- This will show `PropertiesChanged` signals when the topology is updated.

## License

Copyright (c) 2025 Silicon Labs, Inc.

