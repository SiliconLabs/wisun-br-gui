#!/bin/bash

# Wi-SUN SoC Border Router Agent Service Setup Script

set -e

echo "Setting up Wi-SUN SoC Border Router Agent service..."

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root (use sudo)" 
   exit 1
fi

# Copy service file to systemd directory
echo "Installing systemd service file..."
cp wisun-br-bridge-agent.service /etc/systemd/system/

# Reload systemd and enable service
echo "Configuring systemd service..."
systemctl daemon-reload
systemctl enable wisun-br-bridge-agent.service

echo "Setup complete!"
echo ""
echo "Service management commands:"
echo "  Start service:    sudo systemctl start wisun-br-bridge-agent"
echo "  Stop service:     sudo systemctl stop wisun-br-bridge-agent"
echo "  Service status:   sudo systemctl status wisun-br-bridge-agent"
echo "  View logs:        sudo journalctl -u wisun-br-bridge-agent -f"
echo "  Restart service:  sudo systemctl restart wisun-br-bridge-agent"
echo ""
echo "To remove the service:"
echo "  sudo systemctl stop wisun-br-bridge-agent"
echo "  sudo systemctl disable wisun-br-bridge-agent"
echo "  sudo rm /etc/systemd/system/wisun-br-bridge-agent.service"
echo "  sudo systemctl daemon-reload"