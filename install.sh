#!/bin/bash

set -e

BINARY="build/usb-notifier"
SERVICE_FILE="usb-notifier.service"
ASSET="assets/usb-icon.jpeg"
INSTALL_DIR="/usr/share/usb-notifier"
BIN_DIR="/usr/bin"
SERVICE_DIR="$HOME/.config/systemd/user"

# Ensure the binary was built
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary '$BINARY' not found. Please build the project first (cmake --build build)."
    exit 1
fi

echo "Installing usb-notifier..."

# Install asset
sudo mkdir -p "$INSTALL_DIR"
sudo cp "$ASSET" "$INSTALL_DIR/"

# Install binary
sudo cp "$BINARY" "$BIN_DIR/"
sudo chmod +x "$BIN_DIR/usb-notifier"

# Install systemd user service 
mkdir -p "$SERVICE_DIR"
cp "$SERVICE_FILE" "$SERVICE_DIR/"

# Reload systemd user daemon and enable/start the service
systemctl --user daemon-reload
systemctl --user enable usb-notifier.service
systemctl --user start usb-notifier.service

echo "Done! usb-notifier service is enabled and running."
echo "Check status with: systemctl --user status usb-notifier"
