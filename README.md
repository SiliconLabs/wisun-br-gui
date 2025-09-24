
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
- **File logging**: All logs can be written to a file (default: `/var/log/wisun-soc-br-agent.log`).
- **Configurable logging**: Enable/disable colors, debug, and console/file logging via build defines.

## Logging

- By default, logs are written to the console and to `/var/log/wisun-soc-br-agent.log`.
- You can specify a custom log file path at runtime:
	```bash
	./wisun-soc-br-agent --log /tmp/mylog.txt
	# or
	./wisun-soc-br-agent -l /tmp/mylog.txt
	```
- Log output includes timestamps and log levels (INFO, WARN, ERROR, DEBUG).

### Build Defines

You can control logging features at build time by setting the following defines (e.g., via `-D` in CMake or compiler flags):

- `WS_BR_AGENT_LOG_ENABLE_COLORS` (default: 1) — Enable colored log output in console.
- `WS_BR_AGENT_LOG_ENABLE_DEBUG` (default: 1) — Enable debug log output.
- `WS_BR_AGENT_LOG_ENABLE_CONSOLE_LOG` (default: 1) — Enable logging to console.
- `WS_BR_AGENT_LOG_ENABLE_FILE_LOG` (default: 1) — Enable logging to file.

Example (CMake):
```bash
cmake -DWS_BR_AGENT_LOG_ENABLE_COLORS=0 -DWS_BR_AGENT_LOG_ENABLE_DEBUG=0 ..
```

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

