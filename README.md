
# Wi-SUN SoC Border Router Agent

## Overview

Wi-SUN SoC Border### Available Methods

| Method | Input | Output | Description |
|--------|-------|--------|-------------|
| `RestartSoCBorderRouter` | - | - | Restart the border router on the SoC host (performs SoC reset) |
| `StopSoCBorderRouter` | - | - | Stop the border router on the SoC host (Permanent until restart) |
| `SetSoCBorderRouterConfig` | - | - | Apply current configuration to the SoC host | Agent is a Linux-based service for managing and monitoring a Silicon Labs Wi-SUN Border Router. It exposes a TCP server for remote configuration and status queries, and integrates with D-Bus for IPC and system integration. This service is specifically designed for EFR32 Border Router SoC implementations and requires a SiWx917 Wi-Fi module along with a properly configured IPv6 network infrastructure.

The agent service communicates with the [Wi-SUN Border Router GUI](https://github.com/SiliconLabs/wisun-br-gui) application over D-Bus, providing a comprehensive graphical interface for network management, topology visualization, and real-time monitoring.

![Wi-SUN SoC Border Router Agent](assets/wisun-soc-br-agent.png)

## Architecture & Communication

The Wi-SUN SoC Border Router Agent acts as an intermediary between the EFR32 SoC host and external clients, providing both TCP and D-Bus interfaces for comprehensive network management.

```
┌─────────────────┐     ┌──────────────────────┐     ┌─────────────────┐
│   EFR32 SoC     │────▶│  Wi-SUN BR Agent     │────▶│   D-Bus Clients │
│   (Host)        │     │                      │     │  (System Apps)  │
│  Port 11501     │     │  ┌─────────────────┐ │     │                 │
│ • Network Cfg   │◀────┤  │   TCP Server    │ │     │ • Properties    │
│ • Topology      │     │  │   (Port 11500)  │ │◀────┤ • Monitoring    │
│ • Status        │     │  └─────────────────┘ │     │ • Integration   │
│                 │     │                      │     │                 │
└─────────────────┘     │  ┌─────────────────┐ │     └─────────────────┘
                        │  │  D-Bus Service  │ │
┌─────────────────┐     │  │ (SystemD Bus)   │ │
│ Wi-SUN Border   │────▶│  └─────────────────┘ │
│ Router GUI      │     │                      │
│                 │     └──────────────────────┘
│ • Network Mgmt  │
│ • Topology View │
│ • Real-time Mon │
│ • Configuration │
└─────────────────┘

```

### Communication Flow
1. **SoC Host ↔ BR Agent**: TCP communication for network configuration and topology status
2. **GUI ↔ BR Agent**: D-Bus interface for remote management and real-time monitoring
3. **BR Agent → D-Bus**: Property exposure and change notifications for system integration

### Purpose

- Provide a remote management interface for Wi-SUN Border Routers.
- Enable network configuration, topology monitoring, and integration with UI tools (e.g., wisun-br-gui).
- Support automated and scriptable testing via Python and Linux utilities.

## D-Bus Interface

The agent exposes a comprehensive D-Bus interface at `com.silabs.Wisun.BorderRouter` for system integration and monitoring.

### Available Properties

| Property | Type | Description |
|----------|------|-------------|
| `RoutingGraph` | `a(aybaay)` | Network topology with target, preferred, and backup routes |
| `WisunNetworkName` | `s` | Wi-SUN network name/identifier |
| `WisunSize` | `s` | Network size configuration (Small/Medium/Large) |
| `WisunDomain` | `s` | Regulatory domain setting |
| `WisunPhyModeId` | `u` | PHY mode identifier for FAN 1.1 |
| `WisunChanPlanId` | `u` | Channel plan identifier for FAN 1.1 |
| `WisunFanVersion` | `y` | FAN version (1 for FAN 1.0, 2 for FAN 1.1) |

### Available Methods

| Method | Input | Output | Description |
|--------|-------|--------|-------------|
| `RestartSoCBorderRouter` | - | - | Restart the border router: stops FAN 1.1 network, reconfigures, and starts again |
| `StopSoCBorderRouter` | - | - | Stop the border router on the SoC host (Permanent until manual SoC CLI restart) |
| `SetSoCBorderRouterConfig` | - | - | Apply current configuration to the SoC host |

### D-Bus Features

- **Property Monitoring**: All properties support `PropertiesChanged` signals
- **Method Calls**: Control border router operation via D-Bus methods
- **System Integration**: Native systemd D-Bus integration for service management
- **Scripting Support**: Query properties and call methods via `dbus-send` or `busctl` commands
- **Real-time Updates**: Automatic topology change notifications via D-Bus signals

## Features

- TCP server for remote management and configuration
- D-Bus IPC interface for system integration and property signaling
- Thread-safe host and topology management
- Structured message protocol for configuration and topology
- Integration with Silicon Labs Wi-SUN SoC platforms
- Designed for use with graphical UI (wisun-br-gui) and automated scripts
- **File logging**: All logs can be written to a file (default: `/var/log/wisun-soc-br-agent.log`).
- **Configurable logging**: Enable/disable colors, debug, and console/file logging via build defines.
- **Configuration file support**: Load Wi-SUN settings from configuration files
- **Host configuration**: Specify SoC host address via command line. The application updates the given Border Router SoC host automatically, if it's connected to the Wi-Fi network

## Configuration

The agent supports flexible configuration through command-line arguments and configuration files.

### Command Line Arguments

```bash
sudo ./wisun-soc-br-agent [OPTIONS]
```

**Available Options:**
- `--config <file>` or `-c <file>`: Load Wi-SUN settings from configuration file
- `--host <address>` or `-h <address>`: Set EFR32 SoC host IPv6 address (optional, updates remote host configuration)
- `--log <file>` or `-l <file>`: Specify custom log file path

**Examples:**
```bash
# Load settings from config file
sudo ./wisun-soc-br-agent --config /etc/wisun/network.conf

# Optionally set SoC host address
sudo ./wisun-soc-br-agent --host fd12:3456::1

# Combined usage
sudo ./wisun-soc-br-agent --config /etc/wisun/network.conf --host fd12:3456::1 --log /var/log/wisun.log
```

### Configuration Files

Configuration files use simple `key=value` format. See the `config/` directory in this repository for reference configuration files and supported parameters.

## Logging

- By default, logs are written to the console and to `/var/log/wisun-soc-br-agent.log`.
- You can specify a custom log file path at runtime:
	```bash
	sudo ./wisun-soc-br-agent --log /tmp/mylog.txt
	# or
	sudo ./wisun-soc-br-agent -l /tmp/mylog.txt
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

## Testing & Scripts

### 1. Python Bot Test Session (`br_soc_bot.py`)

The Python bot provides comprehensive TCP client simulation with multi-threaded operations:

```bash
python3 test/br_soc_bot.py
```

**Features:**
- Multi-threaded TCP client simulation
- Random topology generation and updates
- Configuration requests and responses
- Structured message protocol testing
- Concurrent client session testing

### 2. D-Bus Property Scripts

#### Query All Settings (`dbus-get-settings.sh`)
```bash
sudo bash test/dbus-get-settings.sh
```
Retrieves and displays all Wi-SUN configuration properties:
- Network name, size, and regulatory domain
- PHY mode and channel plan identifiers
- FAN version information

#### Query Network Topology (`dbus-get-topology.sh`)
```bash
sudo bash test/dbus-get-topology.sh
```
Fetches the current network routing graph with target and route information.

#### Monitor Property Changes (`dbus-monitor-routinggraph.sh`)
```bash
sudo bash test/dbus-monitor-routinggraph.sh
```
Real-time monitoring of `PropertiesChanged` signals for topology updates.

### 3. D-Bus Method Control Scripts

The agent provides convenient shell scripts for controlling border router operations via D-Bus methods:

#### Restart Border Router (`dbus-restart-br.sh`)
```bash
sudo bash test/dbus-restart-br.sh
```
Sends a restart command to the SoC host to stop the FAN 1.1 network, reconfigure, and start it again.

#### Stop Border Router (`dbus-stop-br.sh`)
```bash
sudo bash test/dbus-stop-br.sh
```
Sends a stop command to the SoC host to halt border router operation. **Note**: Once stopped, the effect is permanent until the network is manually started via SoC CLI directly.

#### Set Configuration (`dbus-set-config.sh`)
```bash
sudo bash test/dbus-set-config.sh
```
Applies the current Wi-SUN configuration settings to the SoC host.

### 4. Manual D-Bus Testing

#### Property Queries
Query individual properties using `dbus-send`:
```bash
# Get network name
sudo dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter \
  /com/silabs/Wisun/BorderRouter org.freedesktop.DBus.Properties.Get \
  string:com.silabs.Wisun.BorderRouter string:WisunNetworkName

# Monitor all property changes
sudo dbus-monitor --system "interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'"
```

#### Method Calls
Call border router control methods directly:
```bash
# Restart border router (stops FAN 1.1 network, reconfigures, and starts again)
sudo dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter \
  /com/silabs/Wisun/BorderRouter com.silabs.Wisun.BorderRouter.RestartSoCBorderRouter

# Stop border router
sudo dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter \
  /com/silabs/Wisun/BorderRouter com.silabs.Wisun.BorderRouter.StopSoCBorderRouter

# Set configuration
sudo dbus-send --system --print-reply --dest=com.silabs.Wisun.BorderRouter \
  /com/silabs/Wisun/BorderRouter com.silabs.Wisun.BorderRouter.SetSoCBorderRouterConfig
```

## License

Copyright (c) 2025 Silicon Labs, Inc.

