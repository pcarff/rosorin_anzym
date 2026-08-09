#!/usr/bin/env python3
# encoding: utf-8

import os
import socket
import psutil
import rclpy
from rclpy.node import Node
from std_msgs.msg import UInt16
from ros_robot_controller_msgs.msg import OLEDState

class OLEDInfoNode(Node):
    def __init__(self):
        super().__init__('oled_info_node')
        self.oled_pub = self.create_publisher(OLEDState, '/ros_robot_controller/set_oled', 10)
        self.battery_sub = self.create_subscription(UInt16, '/ros_robot_controller/battery', self.battery_callback, 10)
        self.battery_voltage_str = "Bat:N/A"
        
        # Timer to update OLED display every 2 seconds
        self.timer = self.create_timer(2.0, self.update_display)
        self.get_logger().info('OLED Info Node initialized - updating display every 2s')

    def battery_callback(self, msg):
        # msg.data is in millivolts (e.g., 11800 => 11.8V)
        volts = msg.data / 1000.0
        self.battery_voltage_str = f"Bat:{volts:.1f}V"

    def get_ip_address(self):
        """Fetch active IPv4 address (e.g. 192.168.8.162)"""
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except Exception:
            return "127.0.0.1"

    def get_cpu_temp(self):
        """Read CPU temperature in Celsius"""
        try:
            with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
                temp_c = float(f.read().strip()) / 1000.0
                return f"{temp_c:.0f}C"
        except Exception:
            return "N/A"

    def publish_line(self, line_index, text):
        msg = OLEDState()
        msg.index = line_index
        msg.text = text[:16]  # OLED line capacity is ~16 characters
        self.oled_pub.publish(msg)

    def update_display(self):
        ip_addr = self.get_ip_address()
        cpu_temp = self.get_cpu_temp()

        # Line 1: Robot Status / Name
        self.publish_line(1, "ROSOrin: Online")
        # Line 2: Active IP Address
        self.publish_line(2, f"IP:{ip_addr}")
        # Line 3: Network interface or Hostname
        self.publish_line(3, f"Host:{socket.gethostname()}")
        # Line 4: Battery Voltage & CPU Temp
        self.publish_line(4, f"{self.battery_voltage_str} {cpu_temp}")

def main(args=None):
    rclpy.init(args=args)
    node = OLEDInfoNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
