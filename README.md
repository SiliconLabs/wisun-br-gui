
# Wi-SUN Border Router Bridge Agent

## Overview

Wi-SUN Border Router Bridge Agent is a Linux-based service for managing and monitoring a Silicon Labs Wi-SUN SoC Border Router. 
It exposes a TCP server for remote configuration and status queries, and integrates with D-Bus for IPC and system integration. 
This service is specifically designed for EFR32 Border Router SoC implementations and requires a SiWx917 Wi-Fi module along with a properly configured IPv6 network infrastructure. 
*Wi-SUN Border Router Bridge Agent* SiSDK sample application provides the SoC example that operates with this service. 

The agent service communicates with the [Wi-SUN Border Router GUI](https://github.com/SiliconLabs/wisun-br-gui) application over D-Bus, providing a comprehensive graphical interface for network management, topology visualization, and real-time monitoring.

![Wi-SUN Border Router Bridge Agent](assets/wisun-br-bridge-agent.png)

## Architecture & Communication

The Wi-SUN Border Router Bridge Agent acts as an intermediary between the EFR32 SoC host and external clients, providing both TCP and D-Bus interfaces for comprehensive network management.

```
┌─────────────────┐     ┌──────────────────────┐     ┌─────────────────┐
│   EFR32 SoC     │────>│  Wi-SUN BR Agent     │────>│   D-Bus Clients │
│   (Host)        │     │                      │     │  (System Apps)  │
│  Port 11501     │     │  ┌─────────────────┐ │     │                 │
│ • Network Cfg   │<────┤  │   TCP Server    │ │     │ • Properties    │
│ • Topology      │     │  │   (Port 11500)  │ │<────┤ • Monitoring    │
│ • Status        │     │  └─────────────────┘ │     │ • Integration   │
│                 │     │                      │     │                 │
└─────────────────┘     │  ┌─────────────────┐ │     └─────────────────┘
                        │  │  D-Bus Service  │ │
┌─────────────────┐     │  │ (SystemD Bus)   │ │
│ Wi-SUN Border   │────>│  └─────────────────┘ │
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

The agent exposes a comprehensive D-Bus interface at `com.silabs.Wisun.SocBorderRouterAgent` for system integration and monitoring.

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
| `WisunPanId` | `q` | Personal Area Network ID (16-bit identifier) |
| `WisunClass` | `u` | Wi-SUN operating class for FAN 1.0|
| `WisunMode` | `u` | Wi-SUN operating mode for FAN 1.0|


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
- **File logging**: All logs can be written to a file (default: `/var/log/wisun-br-bridge-agent.log`).
- **Configurable logging**: Enable/disable colors, debug, and console/file logging via build defines.
- **Configuration file support**: Load Wi-SUN settings from configuration files
- **Host configuration**: Specify SoC host address via command line. The application updates the given Border Router SoC host automatically, if it is connected to the Wi-Fi network

## Configuration

The agent supports flexible configuration through command-line arguments and configuration files.

### Command Line Arguments

```bash
sudo wisun-br-bridge-agent [OPTIONS]
```

**Available Options:**
- `--config <file>` or `-c <file>`: Load Wi-SUN settings from configuration file
- `--soc <address>` or `-s <address>`: Set EFR32 SoC host IPv6 address (optional, updates remote host configuration)
- `--log <file>` or `-l <file>`: Specify custom log file path
- `--help` or `-h`: Show help and exit
- `--version` or `-v`: Show version information and exit

**Examples:**
```bash
# Load settings from config file
sudo wisun-br-bridge-agent --config /etc/wisun-br-bridge-agent/ws-soc-br-agent.conf

# Optionally set SoC host address
sudo wisun-br-bridge-agent --soc fd12:3456::1

# Combined usage
sudo wisun-br-bridge-agent --config /etc/wisun-br-bridge-agent/ws-soc-br-agent.conf --soc fd12:3456::1 --log /var/log/wisun-br-bridge-agent.log
```

### Configuration Files

Configuration files use simple `key=value` format. See the `config/` directory in this repository for reference configuration files and supported parameters.

## Logging

- By default, logs are written to the console and to `/var/log/wisun-br-bridge-agent.log`.
- You can specify a custom log file path at runtime:
	```bash
	sudo wisun-br-bridge-agent --log /tmp/mylog.txt
	# or
	sudo wisun-br-bridge-agent -l /tmp/mylog.txt
	```
- Log output includes timestamps and log levels (INFO, WARN, ERROR, DEBUG).

### Build Defines

You can control logging features at build time by setting the following defines (e.g., via `-D` in CMake or compiler flags):

- `WS_BR_AGENT_LOG_ENABLE_COLORS` (default: 1) — Enable colored log output in console.
- `WS_BR_AGENT_LOG_ENABLE_DEBUG` (default: 0) — Enable debug log output.
- `WS_BR_AGENT_LOG_ENABLE_CONSOLE_LOG` (default: 1) — Enable logging to console.
- `WS_BR_AGENT_LOG_ENABLE_FILE_LOG` (default: 1) — Enable logging to file.

Example (CMake):
```bash
cmake -DWS_BR_AGENT_LOG_ENABLE_COLORS=0 -DWS_BR_AGENT_LOG_ENABLE_DEBUG=0 ..
```

## Build and Installation

### Prerequisites

Install required dependencies:
- **Ubuntu/Debian**: `sudo apt install build-essential cmake libsystemd-dev`
- **RHEL/CentOS/Fedora**: `sudo yum install gcc cmake systemd-devel` (or `dnf`)

### Build Steps

1. Clone and build the project:
   ```bash
   git clone <repository-url>
   cd wisun-br-gui
   cd wisun-br-bridge-agent
   mkdir build && cd build
   cmake ..
   make
   ```

2. For development/testing, the binary is located in `build/wisun-br-bridge-agent`.

### Installation

Install the agent system-wide:

```bash
# From the build directory
sudo make install
```

This installs:
- **Executable**: `/usr/bin/wisun-br-bridge-agent`
- **Configuration**: `/etc/wisun-br-bridge-agent/*.conf`
- **Manual page**: `/usr/share/man/man1/wisun-br-bridge-agent.1`

### Custom Installation Prefix

To install to a custom location:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/wisun ..
make
sudo make install
```

### Accessing Documentation

After installation, view the manual page:

```bash
man wisun-br-bridge-agent
```

The manual page contains detailed information about:
- Command-line options and usage examples
- D-Bus interface properties and methods
- Configuration file locations
- Practical D-Bus command examples

## Service Setup

The Wi-SUN Border Router Bridge Agent can be configured as a systemd service for automatic startup and management.

### Service Installation

After installing the binary and configuration files, set up the systemd service:

```bash
# Run the setup script
cd misc
sudo ./setup-service.sh
```

The setup script will:
- Create a dedicated `wisun` user and group for security
- Set up proper permissions for log and configuration directories
- Install and enable the systemd service
- Configure security policies

### Service check

```bash
 $ sudo systemctl list-units 'wisun*'
  UNIT                          LOAD      ACTIVE SUB     DESCRIPTION
  wisun-br-bridge-agent.service loaded    active running Wi-SUN Border Router Bridge Agent
```

## Checking Wi-Fi connection to the SoC Border Router

- Retrieving the SoC Border Router Wifi IPv6 (SoC Border Router CLI console)

```text
> wisun get wifi.ipv6_address
wifi.ipv6_address = 2001:db8:0:2:eef6:4cff:fea0:4320
>
```

- Pinging the SoC Border Router over WiFI (Agent bash console)

```bash
 $ ping 2001:db8:0:2:eef6:4cff:fea0:4320
PING 2001:db8:0:2:eef6:4cff:fea0:4320(2001:db8:0:2:eef6:4cff:fea0:4320) 56 data bytes
64 bytes from 2001:db8:0:2:eef6:4cff:fea0:4320: icmp_seq=1 ttl=255 time=12.2 ms
64 bytes from 2001:db8:0:2:eef6:4cff:fea0:4320: icmp_seq=2 ttl=255 time=13.8 ms
```

## Connecting the SoC Border Router to the Agent

To send updates to the agent, the Soc Border Router needs to know the IPv6 address of the agent, which is the `wlan0` IPv6 address of the host running the agent.
This address needs to be set in the SoC Border Router CLI

- Retrieving the Agent's IPv6 (Agent bash console)

  ```bash
  $ ip address show wlan0 | grep mngtmpaddr
      inet6 2001:db8:0:2:8c2:a572:6130:1941/64 scope global dynamic mngtmpaddr noprefixroute
  ```

- Checking the Agent's IPv6 as seen by the Soc Border Router (Soc Border Router CLI console)

  ```Text
  > wisun get_br_agent_remote_addr
  [2001:DB8:0:2:8C2:A572:6130:1941]
  ```

- (if needed) Setting the Agent's IPv6 as seen by the Soc Border Router (Soc Border Router CLI console)

  ```Text
  > wisun set_br_agent_remote_addr 2001:DB8:0:2:8C2:A572:6130:1941
  [Remote address is set to: 2001:DB8:0:2:8C2:A572:6130:1941]
  ```

> Stopping/starting the SoC Border Router is required to apply the Agent's IPv6

  ```Text
  > wisun stop
  > wisun start_fan11
  [Using built-in trusted CA #0]
  [Using built-in device credentials]
  [Border router PAN ID: 0xFD22]
  [Border router started]
  >
  ```

## Testing & Scripts

### 1. D-Bus Property Scripts

The scripts listed in examples below need to be called from `~/wisun-br-gui/wisun-br-bridge-agent`, 
unless the `~/wisun-br-gui/wisun-br-bridge-agent/test` path is added to `$PATH`, 
adding `export PATH=$PATH:~/wisun-br-gui/wisun-br-bridge-agent/test` to `~/.bashrc`.

```bash
echo "PATH=$PATH:~/wisun-br-gui/wisun-br-bridge-agent/test" >> ~/.bashrc
source ~/.bashrc
```

> Once the PATH is filled, direct access to the scripts is possible from any directory.

#### Query All Settings (`dbus-get-settings.sh`)
```bash
sudo bash test/dbus-get-settings.sh
```
Retrieves and displays all Wi-SUN configuration properties:
- Network name, size, and regulatory domain
- PHY mode and channel plan identifiers
- FAN version information

#### Query Network Topology ([dbus-get-topology.sh](test/dbus-get-topology.sh))

```bash
sudo bash test/dbus-get-topology.sh
```
Fetches the current network routing graph with target and route information.

#### Monitor Property Changes ([dbus-monitor-routinggraph.sh](test/dbus-monitor-routinggraph.sh))

```bash
sudo bash test/dbus-monitor-routinggraph.sh
```
Real-time monitoring of `PropertiesChanged` signals for topology updates.

### 4. Manual D-Bus Testing

#### Identifying D-Bus Wisun instances

```bash
 $ busctl list | grep Wisun
com.silabs.Wisun.SocBorderRouterAgent  4653 wisun-br-bridge root             :1.87         wisun-br-bridge-agent.service -       -
```

#### Instrospection

```bash
$ sudo busctl introspect com.silabs.Wisun.SocBorderRouterAgent /com/silabs/Wisun/SocBorderRouterAgent
NAME                                  TYPE      SIGNATURE RESULT/VALUE                             FLAGS
com.silabs.Wisun.SocBorderRouterAgent interface -         -                                        -
.RestartSoCBorderRouter               method    -         -                                        -
.SetSoCBorderRouterConfig             method    -         -                                        -
.StopSoCBorderRouter                  method    -         -                                        -
.RoutingGraph                         property  a(aybaay) 1 16 253 18 52 86 0 0 0 0 98 164 35 255… emits-invalidation
.WisunChanPlanId                      property  u         32                                       emits-change
.WisunClass                           property  u         0                                        emits-change
.WisunDomain                          property  s         "EU"                                     emits-change
.WisunFanVersion                      property  y         2                                        emits-change
.WisunMode                            property  u         0                                        emits-change
.WisunNetworkName                     property  s         "Wi-SUN Network"                         emits-change
.WisunPanId                           property  q         64802                                    emits-change
.WisunPhyModeId                       property  u         1                                        emits-change
.WisunSize                            property  s         "SMALL"                                  emits-change
org.freedesktop.DBus.Introspectable   interface -         -                                        -
.Introspect                           method    -         s                                        -
org.freedesktop.DBus.Peer             interface -         -                                        -
.GetMachineId                         method    -         s                                        -
.Ping                                 method    -         -                                        -
org.freedesktop.DBus.Properties       interface -         -                                        -
.Get                                  method    ss        v                                        -
.GetAll                               method    s         a{sv}                                    -
.Set                                  method    ssv       -                                        -
.PropertiesChanged                    signal    sa{sv}as  -                                        -
```

#### Property Queries
Query individual properties using `dbus-send`:

```bash
# Get network name
sudo dbus-send --system --print-reply --dest=com.silabs.Wisun.SocBorderRouterAgent \
  /com/silabs/Wisun/SocBorderRouterAgent org.freedesktop.DBus.Properties.Get \
  string:com.silabs.Wisun.SocBorderRouterAgent string:WisunNetworkName

# Monitor all property changes
sudo dbus-monitor --system "interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'"
```

or using `busctl get-property`:

```bash
# Get network name
sudo busctl get-property com.silabs.Wisun.SocBorderRouterAgent  /com/silabs/Wisun/SocBorderRouterAgent \
  com.silabs.Wisun.SocBorderRouterAgent RoutingGraph

# Monitor all property changes
sudo busctl monitor com.silabs.Wisun.SocBorderRouterAgent"
```

(more compact result can be obtained adding `--json=short`)

```bash
# Monitor all property changes
$ sudo busctl monitor com.silabs.Wisun.SocBorderRouterAgent --json=short
Monitoring bus message stream.
{"type":"signal","endian":"l","flags":1,"version":1,"cookie":7,"sender":":1.53","path":"/com/silabs/Wisun/SocBorderRouterAgent","interface":"org.freedesktop.DBus.Properties","member":"PropertiesChanged","payload":{"type":"sa{sv}as","data                                                                                                      ":["com.silabs.Wisun.SocBorderRouterAgent",{"WisunNetworkName":{"type":"s","data":"Wi-SUN Network"},"WisunSize":{"type":"s","data":"SMALL"},"WisunDomain":{"type":"s","data":"EU"},"WisunPhyModeId":{"type":"u","data":1},"WisunChanPlanId":{                                                                                                      "type":"u","data":32},"WisunFanVersion":{"type":"y","data":2}},[]]}}
{"type":"signal","endian":"l","flags":1,"version":1,"cookie":8,"sender":":1.53","path":"/com/silabs/Wisun/SocBorderRouterAgent","interface":"org.freedesktop.DBus.Properties","member":"PropertiesChanged","payload":{"type":"sa{sv}as","data                                                                                                      ":["com.silabs.Wisun.SocBorderRouterAgent",{},["RoutingGraph"]]}}
```

## Limitations / Known issues

- The exposed methods on the D-Bus interface are not tested. You may encounter issues using them.

## License

This project is licensed under the terms specified in the [LICENSE.txt](LICENSE.txt) file.

Copyright (c) 2025 Silicon Laboratories Inc. All rights reserved.

For complete licensing terms and conditions, please refer to the LICENSE.txt file included with this distribution.

## Release Notes

### Version 1.0.0 - Initial Release

**Release Date:** December 2025

**New Features:**
- Initial implementation of Wi-SUN Border Router Bridge Agent
- TCP server for remote configuration and status queries (port 11500)
- Comprehensive D-Bus interface for system integration
- Support for EFR32 SoC Border Router implementations
- Multi-threaded architecture with thread-safe operations
- Configurable logging with file and console output
- Configuration file support for Wi-SUN network settings
- Systemd service integration with optional setup scripts
- Manual page documentation and installation support

**D-Bus Interface:**
- Properties: RoutingGraph, WisunNetworkName, WisunSize, WisunDomain, WisunPhyModeId, WisunChanPlanId, WisunFanVersion, WisunPanId, WisunClass, WisunMode
- Real-time property change notifications via PropertiesChanged signals

**Testing & Development Tools:**
- D-Bus property query and monitoring scripts  
- Comprehensive example configurations and documentation

