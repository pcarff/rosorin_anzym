#!/usr/bin/env python3
# encoding: utf-8
"""
Teleop Mode Switcher & Zero-Latency Command Router for AnZym_ROSOrin.

Multiplexes velocity commands (/controller/cmd_vel) between:
 1. LOCAL: Onboard wireless joystick controller (/controller/cmd_vel_local)
 2. GCS_REMOTE: GCS Bluetooth/Web joystick teleop over rosbridge (/gcs/cmd_vel)

Optimization:
 - Queue size depth=1 to eliminate buffering latency.
 - Non-blocking asynchronous threads for background beacon registration.
 - Periodic status publisher for GCS synchronization.

Fail-Safe Behavior:
 If GCS connection or heartbeat stops for >3 seconds while in GCS_REMOTE mode,
 the node automatically halts the robot (zero Twist), reverts to LOCAL mode,
 and triggers an audible buzzer alert.
"""

import os
import time
import socket
import json
import threading
import urllib.request
import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from geometry_msgs.msg import Twist
from std_msgs.msg import String
from std_srvs.srv import SetBool
from ros_robot_controller_msgs.msg import BuzzerState


class TeleopModeSwitcher(Node):
    def __init__(self):
        super().__init__('teleop_mode_switcher')

        # Parameters
        self.declare_parameter('default_mode', 'LOCAL')  # 'LOCAL' or 'GCS_REMOTE'
        self.declare_parameter('heartbeat_timeout_sec', 3.0)
        self.declare_parameter('gcs_host', os.environ.get('GCS_HOST', 'localhost'))

        self.mode = self.get_parameter('default_mode').value.upper()
        self.heartbeat_timeout = float(self.get_parameter('heartbeat_timeout_sec').value)
        self.gcs_host = str(self.get_parameter('gcs_host').value)
        self.last_gcs_msg_time = 0.0

        # Zero-latency publishers
        self.cmd_vel_pub = self.create_publisher(Twist, '/controller/cmd_vel', 1)
        self.buzzer_pub = self.create_publisher(BuzzerState, '/ros_robot_controller/set_buzzer', 1)
        self.mode_status_pub = self.create_publisher(String, '/teleop_mode_status', 1)

        # Subscriptions
        self.local_joy_sub = self.create_subscription(
            Twist, '/controller/cmd_vel_local', self.local_cmd_callback, 1
        )
        self.gcs_cmd_sub = self.create_subscription(
            Twist, '/gcs/cmd_vel', self.gcs_cmd_callback, qos_profile_sensor_data
        )

        # Service to set teleop mode from GCS or local ROS service calls
        self.mode_srv = self.create_service(
            SetBool, '/set_teleop_mode', self.handle_set_teleop_mode
        )

        # Heartbeat watchdog timer (checks every 0.2s)
        self.watchdog_timer = self.create_timer(0.2, self.watchdog_check)

        # Periodic status heartbeat publisher (1 Hz)
        self.status_timer = self.create_timer(1.0, self.publish_status)

        # Non-blocking GCS Auto-Beacon Registration Timer (every 10s in background thread)
        self.beacon_timer = self.create_timer(10.0, self.trigger_async_beacon)

        self.get_logger().info(
            f'Teleop Mode Switcher initialized with zero-latency queue. Default Mode: {self.mode}'
        )

    def publish_status(self):
        """Publish current mode on /teleop_mode_status."""
        msg = String()
        msg.data = self.mode
        self.mode_status_pub.publish(msg)

    def handle_set_teleop_mode(self, request, response):
        """Service callback: data=True -> GCS_REMOTE, data=False -> LOCAL."""
        new_mode = 'GCS_REMOTE' if request.data else 'LOCAL'
        self.mode = new_mode
        self.last_gcs_msg_time = time.time()
        self.publish_status()
        
        response.success = True
        response.message = f'Teleop mode set to {self.mode}'
        self.get_logger().info(response.message)

        # Sound buzzer notification non-blockingly
        self.trigger_buzzer(freq=3000 if request.data else 1500, repeat=1)
        return response

    def local_cmd_callback(self, msg: Twist):
        """Forward local joystick commands immediately when in LOCAL mode."""
        if self.mode == 'LOCAL':
            self.cmd_vel_pub.publish(msg)

    def gcs_cmd_callback(self, msg: Twist):
        """Forward GCS commands immediately when in GCS_REMOTE mode and update heartbeat."""
        self.last_gcs_msg_time = time.time()
        if self.mode == 'GCS_REMOTE':
            self.cmd_vel_pub.publish(msg)

    def watchdog_check(self):
        """Check GCS heartbeat timeout. Fallback to LOCAL mode and stop robot if GCS drops."""
        if self.mode == 'GCS_REMOTE':
            elapsed = time.time() - self.last_gcs_msg_time
            if elapsed > self.heartbeat_timeout:
                self.get_logger().warn(
                    f'GCS teleop heartbeat timeout ({elapsed:.1f}s > {self.heartbeat_timeout}s). '
                    f'Reverting to LOCAL direct joystick mode and halting motors!'
                )
                self.mode = 'LOCAL'
                # Publish safe halt
                stop_twist = Twist()
                self.cmd_vel_pub.publish(stop_twist)
                self.publish_status()
                self.trigger_buzzer(freq=1000, repeat=2)

    def trigger_async_beacon(self):
        """Spawn background worker thread for GCS registration to prevent event loop stalls."""
        threading.Thread(target=self._beacon_worker, daemon=True).start()

    def _beacon_worker(self):
        """Non-blocking HTTP registration attempt."""
        try:
            my_ip = self.get_local_ip()
            hostname = socket.gethostname()
            robot_id = hostname.lower().replace('-', '_')

            url = f'http://{self.gcs_host}:8000/api/robots/register'
            payload = json.dumps({
                "robot_id": robot_id,
                "robot_name": f"Robot {hostname}",
                "host": my_ip,
                "port": 9090
            }).encode('utf-8')

            req = urllib.request.Request(url, data=payload, headers={'Content-Type': 'application/json'})
            with urllib.request.urlopen(req, timeout=0.5) as resp:
                if resp.status == 200:
                    self.get_logger().debug(f'Auto-registered with GCS at {url}')
        except Exception:
            pass  # Silent non-blocking fail

    def get_local_ip(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except Exception:
            return "127.0.0.1"

    def trigger_buzzer(self, freq=2000, repeat=1):
        """Utility to sound STM32 buzzer state."""
        try:
            msg = BuzzerState()
            msg.freq = freq
            msg.on_time = 0.08
            msg.off_time = 0.05
            msg.repeat = repeat
            self.buzzer_pub.publish(msg)
        except Exception:
            pass


def main(args=None):
    rclpy.init(args=args)
    node = TeleopModeSwitcher()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, rclpy.executors.ExternalShutdownException):
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
