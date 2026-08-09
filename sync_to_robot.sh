#!/bin/bash
# ==============================================================================
# Workstation Deploy Script (Runs on Development Workstation ONLY)
# Syncs ONLY active ROS2 package code (src/) and robot scripts to the robot.
# ==============================================================================

REMOTE_HOST="rosorin"
REMOTE_DIR="~/anzym_robot_ws"
LOCAL_DIR="/home/pcarff/Workspaces/AnZym_ROSOrin"

# Color output helpers
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}====================================================${NC}"
echo -e "${BLUE}   Deploying Active Code to Robot (${REMOTE_HOST})   ${NC}"
echo -e "${BLUE}====================================================${NC}"

# Ensure target directories exist on robot
ssh "${REMOTE_HOST}" "mkdir -p ${REMOTE_DIR}/src ${REMOTE_DIR}/scripts"

# 1. Sync src/ (ROS2 source code packages)
echo -e "${YELLOW}--> Syncing active ROS2 packages (src/)...${NC}"
rsync -avz --delete \
    --exclude='__pycache__/' \
    --exclude='*.pyc' \
    --exclude='.DS_Store' \
    "${LOCAL_DIR}/src/" "${REMOTE_HOST}:${REMOTE_DIR}/src/"

# 2. Sync scripts/ (Robot-side helper & bringup scripts)
echo -e "${YELLOW}--> Syncing robot helper scripts (scripts/)...${NC}"
rsync -avz --delete \
    --exclude='__pycache__/' \
    "${LOCAL_DIR}/scripts/" "${REMOTE_HOST}:${REMOTE_DIR}/scripts/"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✔ Code sync complete! (Excluded HiWonder/ and workstation-only tools)${NC}"
else
    echo -e "${RED}✘ Sync failed! Check SSH connection to ${REMOTE_HOST}.${NC}"
    exit 1
fi

# 3. Trigger remote colcon build on robot unless --no-build flag is passed
if [[ "$1" != "--no-build" ]]; then
    echo -e "${YELLOW}--> Compiling workspace on robot...${NC}"
    ssh "${REMOTE_HOST}" "bash -c 'chmod +x ${REMOTE_DIR}/scripts/*.sh && ${REMOTE_DIR}/scripts/build.sh'"
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✔ Remote colcon build succeeded on robot!${NC}"
    else
        echo -e "${RED}✘ Remote colcon build failed on robot.${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}====================================================${NC}"
echo -e "${GREEN}   Deployment Finished Successfully!                ${NC}"
echo -e "${GREEN}====================================================${NC}"
