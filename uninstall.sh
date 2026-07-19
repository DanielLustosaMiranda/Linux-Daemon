#!/bin/bash

set -e

INSTALL_DIR="/usr/share/usb-notifier"
BIN_PATH="/usr/bin/usb-notifier"
SERVICE_NAME="usb-notifier.service"
SERVICE_PATH="$HOME/.config/systemd/user/$SERVICE_NAME"

echo "Uninstalling usb-notifier..."

# Stop and disable the systemd user service if it exists
if systemctl --user is-active --quiet "$SERVICE_NAME" 2>/dev/null; then
    systemctl --user stop "$SERVICE_NAME"
fi

if systemctl --user is-enabled --quiet "$SERVICE_NAME" 2>/dev/null; then
    systemctl --user disable "$SERVICE_NAME"
fi

# Remove the service file
if [ -f "$SERVICE_PATH" ]; then
    rm -f "$SERVICE_PATH"
    systemctl --user daemon-reload
fi

# Remove the binary
if [ -f "$BIN_PATH" ]; then
    sudo rm -f "$BIN_PATH"
fi

# Remove the asset directory
if [ -d "$INSTALL_DIR" ]; then
    sudo rm -rf "$INSTALL_DIR"
fi

echo "Done! usb-notifier has been removed."
