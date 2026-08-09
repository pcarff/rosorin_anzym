#!/bin/bash
# ==============================================================================
# Setup Autostart Service on Robot (Runs on Robot)
# Enables/disables systemd service to run bringup_robot.sh automatically on boot.
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVICE_NAME="rosorin_bringup.service"
SERVICE_SRC="$SCRIPT_DIR/$SERVICE_NAME"
SERVICE_DEST="/etc/systemd/system/$SERVICE_NAME"

ACTION="${1:-install}"

case "$ACTION" in
    install|enable)
        echo "Installing $SERVICE_NAME from $SERVICE_SRC..."
        if [ ! -f "$SERVICE_SRC" ]; then
            echo "Error: $SERVICE_SRC not found!"
            exit 1
        fi
        sudo cp "$SERVICE_SRC" "$SERVICE_DEST"
        sudo chmod 644 "$SERVICE_DEST"
        sudo systemctl daemon-reload
        sudo systemctl enable "$SERVICE_NAME"
        echo "Starting service..."
        sudo systemctl restart "$SERVICE_NAME"
        echo "✔ $SERVICE_NAME installed, enabled, and started!"
        echo ""
        sudo systemctl status "$SERVICE_NAME" --no-pager || true
        ;;
    uninstall|disable|remove)
        echo "Disabling and removing $SERVICE_NAME..."
        sudo systemctl stop "$SERVICE_NAME" || true
        sudo systemctl disable "$SERVICE_NAME" || true
        sudo rm -f "$SERVICE_DEST"
        sudo systemctl daemon-reload
        echo "✔ $SERVICE_NAME uninstalled!"
        ;;
    status)
        sudo systemctl status "$SERVICE_NAME" --no-pager
        ;;
    start)
        sudo systemctl start "$SERVICE_NAME"
        sudo systemctl status "$SERVICE_NAME" --no-pager
        ;;
    stop)
        sudo systemctl stop "$SERVICE_NAME"
        echo "Service stopped."
        ;;
    *)
        echo "Usage: $0 {install|enable|uninstall|disable|status|start|stop}"
        exit 1
        ;;
esac
