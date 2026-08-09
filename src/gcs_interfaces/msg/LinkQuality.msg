# LinkQuality Message
# Telemetry link quality metrics for UAS communication channel monitoring.
#
# Published by: Vehicle-side telemetry node
# Subscribed by: GCS display, link failsafe logic

int8 rssi                              # Signal strength in dBm (negative values, e.g. -45 to -90)
float32 packet_loss_rate               # Percentage of lost packets (0.0 to 100.0)
uint32 bytes_sent                      # Cumulative bytes transmitted from GCS to vehicle
uint32 bytes_received                  # Cumulative bytes received from vehicle to GCS
float32 jitter                         # Network latency jitter in milliseconds