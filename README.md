<table border="0">
  <tr>
    <td align="left" valign="middle">
    <h1>Wi-SUN Border Router Graphical User Interface</h1>
  </td>
  <td align="left" valign="middle">
    <a href="https://wi-sun.org/">
      <img src="pics/wisun-logo.png" title="Wi-SUN" alt="Wi-SUN Logo" width="300"/>
    </a>
  </td>
  </tr>
</table>

The Wi-SUN border router GUI is a companion tool for Silicon Labs' Wi-SUN border router reference design, [wsbrd](https://github.com/SiliconLabs/wisun-br-linux). The tool helps manage the network by enabling a remote border router configuration and provides a visual representation of the connected Wi-SUN devices.

# Quick Start Guide

## Prerequisites

To access the graphical interface, a running wsbrd instance is necessary. For more details on the Linux border router installation, follow the [Wi-SUN Linux Border Router Quick Start Guide](https://github.com/SiliconLabs/wisun-br-linux/blob/main/README.md).

Once the border router is successfully installed, [Cockpit](https://cockpit-project.org/) must be installed. Cockpit's installation depends on the operating system used. On Debian-based OSes, use the command below.

```
sudo apt-get install cockpit
```

## Installation

Cockpit features can be extended by adding plugins. Installing a plugin means copying the files of the plugin into the corresponding directory. There are multiple locations where Cockpit searches for plugins. The easiest method is to install them in the user's home directory, under `~/.local/share/cockpit` (for most Raspberry PI installations the full path will be `/home/pi/.local/share/cockpit`). If it doesn't already exist, this directory hierarchy needs to be created. If the directory exists, the plugins have to be downloaded (or cloned with `git`).

Currently, the Wi-SUN border router GUI consists of two plugins:
 - **Wi-SUN Dashboard**: border router configuration and status
 - **Wi-SUN Topology**: network topology and Wi-SUN node information

These plugins share some common files thus their files are structured into three directories: `wisun_dashboard`, `wisun_topology`, and `wisun_common`. These directories must be copied to Cockpit's directory, you can use the following commands to copy the folders:

```
mkdir -p ~/.local/share/cockpit
cd wisun-br-web-interface/
cp -R wisun_dashboard ~/.local/share/cockpit/
cp -R wisun_topology ~/.local/share/cockpit/
cp -R wisun_common ~/.local/share/cockpit/
```

Then, your `~/.local/share/cockpit/` would look like the following:
```
home/
└pi/
 └.local/
  └share/
   └cockpit/
    └wisun_common/
    └wisun_dashboard/
    └wisun_topology/
```

To list the Cockpit packages installed, use the command below.

```
$ cockpit-bridge --packages
...
wisun_dashboard       Wi-SUN Dashbord                          ~/.local/share/cockpit/wisun_dashboard
wisun_topology       Wi-SUN Topology                          /~/.local/share/cockpit/wisun_topology
...
```

The wsbrd configuration file needs to be copied to `/etc` in order to launch the service using systemd:

```
sudo cp /usr/local/share/doc/wsbrd/examples/wsbrd.conf /etc
```

Once the files are copied, the plugins are ready to use.

## Getting Started

### Cockpit Launch

Cockpit features are accessible through its Web interface. It is available at `http://[border router server]:9090/`. If the plugins are installed correctly, they should appear in the left side panel of Cockpit's Web interface.

![Cockpit](pics/cockpit.png)

> To be able to use the Wi-SUN GUI, a user should have **Administrative access** to Cockpit. This can be achieved by *turning on Administrative access* from the **Turn on administrative access** button or **limited access** button shown in the image below and entering the pi password.

![administrative_access](pics/administrative_access.png)

### Wi-SUN Dashboard

The Wi-SUN Dashboard provides direct access to `wsbrd.conf` configuration file. It gives the ability to change your wsbrd configuration without the need to physically access the Raspberry Pi. The Wi-SUN Border Router service box allows the user to start, restart or stop the Wi-SUN Border Router service by clicking the three dots dropdown. The other boxes expose the Wi-SUN Border Router active configuration.

![dashboard](pics/dashboard.png)

### Wi-SUN Topology

As its name indicates, the Wi-SUN Topology plugin draws the Wi-SUN network topology. The canvas is dynamic and uses the autozoom to fits the network on the user screen. This functionality can be disabled by unchecking the `auto-zoom` box. The interactive capability of the network gives you the ability to check the properties of every node of the network through the `Device Properties` Box.

![administrative_access](pics/topology.png)

## Known Issues

In rare cases, the Topology plugin may be affected by lags when the Auto zoom feature is enabled.

## Reporting Bugs/Issues and Posting Questions and Comments

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of this repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of this repo.

## Disclaimer

The Gecko SDK suite supports development with Silicon Labs IoT SoC and module devices. Unless otherwise specified in the specific directory, all examples are considered to be EXPERIMENTAL QUALITY which implies that the code provided in the repos has not been formally tested and is provided as-is. It is not suitable for production environments. In addition, this code will not be maintained and there may be no bug maintenance planned for these resources. Silicon Labs may update projects from time to time.