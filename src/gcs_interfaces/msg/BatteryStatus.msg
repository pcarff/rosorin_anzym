# BatteryStatus Message
# Real-time battery pack telemetry for UAS power monitoring.
#
# Published by: Vehicle-side power monitor node
# Subscribed by: GCS display, low-battery failsafe logic

float32 voltage                        # Total pack voltage (V)
float32 current                        # Amperage draw (A); negative value indicates discharging
float32 remaining_percent              # Remaining capacity estimate (0.0 to 100.0)
float32 cell_temperature               # Average cell temperature in Celsius
uint16 health_flags                    # Bitmask for cell health status:
                                       #   Bit 0 (0x0001): Cell voltage imbalance detected
                                       #   Bit 1 (0x0002): Overvoltage on one or more cells
                                       #   Bit 2 (0x0004): Undervoltage on one or more cells
                                       #   Bit 3 (0x0008): Overcurrent protection triggered
                                       #   Bit 4 (0x0010): Temperature out of safe range
                                       #   Bit 5 (0x0020): Critical failure - immediate landing required