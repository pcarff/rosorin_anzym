#!/usr/bin/env python3
"""
Direct hardware test for PWM steering servo.
Bypasses ROS entirely - talks to STM32 board directly via serial.
Run this WHILE bringup is NOT running (it opens /dev/rrc exclusively).
"""
import sys
import os
import glob
import time

ws_dir = os.path.expanduser('~/anzym_robot_ws')
dist_packages = glob.glob(os.path.join(ws_dir, 'install', '*', 'lib', 'python*', 'site-packages')) + \
                glob.glob(os.path.join(ws_dir, 'install', '*', 'lib', 'python*', 'dist-packages'))
for p in dist_packages:
    if p not in sys.path:
        sys.path.insert(0, p)

from ros_robot_controller.ros_robot_controller_sdk import Board

print("=== Direct PWM Servo Hardware Test ===")
print("Opening /dev/rrc serial port...")

try:
    board = Board()
    board.enable_reception()
    print("Board connected successfully!\n")
except Exception as e:
    print(f"ERROR: Could not connect to board: {e}")
    sys.exit(1)

# Test all 4 PWM servo ports
for servo_id in range(1, 5):
    print(f"--- Testing PWM Servo ID {servo_id} ---")
    
    print(f"  Moving servo {servo_id} to position 1200 (left)...")
    board.pwm_servo_set_position(0.5, [[servo_id, 1200]])
    time.sleep(1.5)
    
    print(f"  Moving servo {servo_id} to position 1800 (right)...")
    board.pwm_servo_set_position(0.5, [[servo_id, 1800]])
    time.sleep(1.5)
    
    print(f"  Moving servo {servo_id} to position 1500 (center)...")
    board.pwm_servo_set_position(0.5, [[servo_id, 1500]])
    time.sleep(1.0)
    
    print(f"  Did servo {servo_id} move? (watch all servo connectors on the board)\n")

# Also try reading servo positions
print("--- Attempting to read PWM servo positions ---")
for servo_id in range(1, 5):
    try:
        pos = board.pwm_servo_read_position(servo_id)
        print(f"  Servo {servo_id} position: {pos}")
    except Exception as e:
        print(f"  Servo {servo_id} read error: {e}")

print("\n=== Test Complete ===")
print("If NO servo moved on ANY port: firmware or board issue.")
print("If a servo moved on a DIFFERENT port than expected: it's plugged into the wrong connector.")
