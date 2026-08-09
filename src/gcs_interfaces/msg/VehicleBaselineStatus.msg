# VehicleBaselineStatus Message
# Universal baseline telemetry status for all UAS vehicles in the fleet.
# Every vehicle must publish this message at a minimum rate of 1 Hz.
#
# Published by: Vehicle-side telemetry aggregator node
# Subscribed by: GCS display, fleet manager, failsafe monitor
#
# Flight Mode Constants (uint8):
#   FLIGHT_MODE_STABILIZE   = 0  # Manual attitude control
#   FLIGHT_MODE_ALT_HOLD    = 1  # Manual attitude with altitude hold
#   FLIGHT_MODE_AUTO        = 2  # Full autonomous mission execution
#   FLIGHT_MODE_GUIDED      = 3  # External guidance (GCS command velocity)
#   FLIGHT_MODE_RTL         = 4  # Return-to-home
#   FLIGHT_MODE_LAND        = 5  # Autonomous landing
#   FLIGHT_MODE_TAKEOFF     = 6  # Autonomous takeoff
#   FLIGHT_MODE_OFFBOARD    = 7  # Offboard control mode
#
# Safety Flag Bitmask (uint32):
#   Bit  0 (0x00000001): Geofence boundary breach
#   Bit  1 (0x00000002): RC link loss detected
#   Bit  2 (0x00000004): Low battery warning
#   Bit  3 (0x00000008): GCS communication timeout
#   Bit  4 (0x00000010): Motor/propulsion fault
#   Bit  5 (0x00000020): GPS fix lost or degraded
#   Bit  6 (0x00000040): Airspeed sensor anomaly
#   Bit  7 (0x00000080): Barometer fault
#   Bit  8 (0x00000100): Compass interference detected
#   Bit  9 (0x00000200): Battery critical - land immediately
#   Bit 10 (0x00000400): Motor current overlimit
#   Bit 11 (0x00000800): IMU data anomaly
#   Bit 12 (0x00001000): Mission item execution error
#   Bit 13 (0x00002000): Actuator saturation detected
#   Bit 14 (0x00004000): parachute/escape system armed
#   Bit 15 (0x00008000): parachute/escape system deployed

builtin_interfaces/Time stamp          # Log/sync timestamp from vehicle clock
uint8 drone_id                         # Unique fleet identifier (1-255; 0 = unset)
string vehicle_type                    # Platform type: multirotor, fixed_wing, vtol, rover, etc.
gcs_interfaces/BatteryStatus battery   # Nested battery telemetry
gcs_interfaces/LinkQuality network     # Nested link quality telemetry
uint8 flight_mode                      # Current flight control mode (see constants above)
uint32 safety_flags                    # Bitmask for active failsafe conditions (see bitmask above)
bool rtk_locked                        # Differential GPS (RTK) lock status