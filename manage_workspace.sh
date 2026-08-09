#!/bin/bash
# Universal workspace helper for ROS 2 Jazzy / Qt6 Ground Control & Robot Dev

# --- CONFIGURATION (Adjust these once) ---
ROBOT_ALIAS="rosorin" 
ROBOT_IP="192.168.8.162"
ROBOT_USER="pcarff"
ROS_DISTRO="jazzy"
ROBOT_WS="~/anzym_robot_ws"

set -e

# Color output helpers
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

show_help() {
    echo -e "${BLUE}🛸 Robot & GCS Workspace Manager (${ROS_DISTRO})${NC}"
    echo "Usage: ./manage_workspace.sh [command]"
    echo ""
    echo "Commands:"
    echo "  init      Run this first! Sets up SSH keys, configs, and updates local environments."
    echo "  start     Daily session starter. Verifies connection and prints an AI reminder."
    echo "  deploy    Pushes active packages (src/) and scripts/ to robot and runs remote colcon build."
    echo "  bringup   Launches full joystick teleop and hardware bringup on robot via SSH."
    echo "  status    Checks robot network connectivity and systemd autostart service status."
    echo "  help      Show this menu."
}

do_init() {
    echo -e "${BLUE}⚙️  Initializing Workspace Environment (${ROS_DISTRO})...${NC}"

    # 1. Handle SSH Key Generation
    if [ ! -f "$HOME/.ssh/id_ed25519" ]; then
        echo -e "${YELLOW}🔑 Generating new ED25519 SSH key pair...${NC}"
        ssh-keygen -t ed25519 -C "workstation_to_robot" -N "" -f "$HOME/.ssh/id_ed25519"
    else
        echo -e "${GREEN}✅ SSH key already exists.${NC}"
    fi

    # 2. Inject SSH Config Alias if missing
    if ! grep -q "Host ${ROBOT_ALIAS}" "$HOME/.ssh/config" 2>/dev/null; then
        echo -e "${YELLOW}📝 Configuring SSH alias '${ROBOT_ALIAS}' in ~/.ssh/config...${NC}"
        mkdir -p "$HOME/.ssh"
        touch "$HOME/.ssh/config"
        cat << EOF >> "$HOME/.ssh/config"

Host ${ROBOT_ALIAS}
    HostName ${ROBOT_IP}
    User ${ROBOT_USER}
    IdentityFile ~/.ssh/id_ed25519
    ConnectTimeout 5
EOF
    else
        echo -e "${GREEN}✅ SSH config alias '${ROBOT_ALIAS}' is already set up.${NC}"
    fi

    # 3. Authorize Key on Robot
    echo -e "${YELLOW}📡 Attempting to copy public key to the robot (Enter robot password if prompted)...${NC}"
    if ssh-copy-id -o ConnectTimeout=3 "${ROBOT_ALIAS}" 2>/dev/null; then
        echo -e "${GREEN}✅ SSH key successfully authorized on the robot!${NC}"
    else
        echo -e "${YELLOW}⚠️  Could not push key automatically. Ensure the robot is powered on and reachable at ${ROBOT_IP}.${NC}"
    fi

    # 4. Local Build Check
    echo -e "${YELLOW}📦 Checking local ROS2 environment...${NC}"
    if [ -f "/opt/ros/${ROS_DISTRO}/setup.bash" ]; then
        echo -e "${GREEN}💡 Local ROS2 ${ROS_DISTRO} found! To build locally: source /opt/ros/${ROS_DISTRO}/setup.bash && colcon build${NC}"
    else
        echo -e "${RED}❌ ROS2 ${ROS_DISTRO} not found natively in /opt/ros/${ROS_DISTRO}.${NC}"
    fi
    
    echo -e "${GREEN}🎉 Init complete. You are ready to run: ./manage_workspace.sh start${NC}"
}

do_start() {
    echo -e "${BLUE}🚀 Initializing Daily Coding Session...${NC}"
    
    # Check network ping to robot
    echo -e "${YELLOW}📡 Pinging robot (${ROBOT_ALIAS} / ${ROBOT_IP})...${NC}"
    if ping -c 1 -W 2 "${ROBOT_IP}" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Robot is online and reachable.${NC}"
        
        # Test passwordless SSH connectivity
        if ssh -q -o ConnectTimeout=3 "${ROBOT_ALIAS}" exit; then
            echo -e "${GREEN}✅ SSH Key authentication confirmed.${NC}"
        else
            echo -e "${RED}❌ SSH connection failed. Run './manage_workspace.sh init' to set up keys.${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️  WARNING: Robot is offline or network configuration is incorrect.${NC}"
    fi

    # Visual Reminder Banner for session workflow
    echo -e "\n${BLUE}======================================================================${NC}"
    echo -e "${GREEN}📋 CURRENT SESSION REMINDER:${NC}"
    echo -e "1. Source ROS2 Jazzy:           source /opt/ros/${ROS_DISTRO}/setup.bash"
    echo -e "2. Build local GCS code:        colcon build --packages-select gcs_ui"
    echo -e "3. Push modifications to robot: ./manage_workspace.sh deploy"
    echo -e "4. Launch robot teleop:         ./manage_workspace.sh bringup"
    echo -e "${BLUE}======================================================================${NC}"
    echo -e "${YELLOW}🤖 PROMPT FOR YOUR AI ASSISTANT:${NC}"
    echo -e "   'Please read .github/AI_WORKSPACE_CONTEXT.md before writing code.'"
    echo -e "${BLUE}======================================================================${NC}"
}

do_deploy() {
    echo -e "${BLUE}🔄 Launching deployment pipeline to target robot (${ROBOT_ALIAS}:${ROBOT_WS})...${NC}"
    WORKSPACE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

    # Ensure remote directory structure exists
    ssh "${ROBOT_ALIAS}" "mkdir -p ${ROBOT_WS}/src ${ROBOT_WS}/scripts"

    # 1. Sync src/ (Exclude build, install, log, pycache, and workstation-only gcs_ui if present)
    echo -e "${YELLOW}📦 Syncing ROS2 packages (src/) to ${ROBOT_WS}/src/...${NC}"
    rsync -avz --delete \
        --exclude="build/" \
        --exclude="install/" \
        --exclude="log/" \
        --exclude="__pycache__/" \
        --exclude="*.pyc" \
        --exclude=".DS_Store" \
        --exclude="gcs_ui/" \
        "${WORKSPACE_ROOT}/src/" "${ROBOT_ALIAS}:${ROBOT_WS}/src/"

    # 2. Sync helper & bringup scripts (scripts/)
    echo -e "${YELLOW}🤖 Syncing robot helper scripts (scripts/) to ${ROBOT_WS}/scripts/...${NC}"
    rsync -avz --delete \
        --exclude="__pycache__/" \
        --exclude="*.pyc" \
        "${WORKSPACE_ROOT}/scripts/" "${ROBOT_ALIAS}:${ROBOT_WS}/scripts/"

    # 3. Trigger remote build
    if [[ "$2" != "--no-build" ]]; then
        echo -e "${YELLOW}🛠️  Triggering remote ROS2 colcon build on vehicle...${NC}"
        ssh "${ROBOT_ALIAS}" "chmod +x ${ROBOT_WS}/scripts/*.sh && cd ${ROBOT_WS} && source /opt/ros/${ROS_DISTRO}/setup.bash && colcon build --symlink-install"
        echo -e "${GREEN}✅ Remote deployment and compilation complete!${NC}"
    else
        echo -e "${GREEN}✅ Code sync complete (skipped remote build).${NC}"
    fi
}

do_bringup() {
    echo -e "${BLUE}🕹️ Launching joystick bringup on robot (${ROBOT_ALIAS})...${NC}"
    ssh -t "${ROBOT_ALIAS}" "bash ${ROBOT_WS}/scripts/bringup_joystick.sh"
}

do_status() {
    echo -e "${BLUE}📊 Checking robot status and autostart service...${NC}"
    ssh -t "${ROBOT_ALIAS}" "bash ${ROBOT_WS}/scripts/setup_autostart.sh status"
}

# Router
case "$1" in
    init)    do_init ;;
    start)   do_start ;;
    deploy)  do_deploy "$@" ;;
    bringup) do_bringup ;;
    status)  do_status ;;
    *)       show_help ;;
esac
