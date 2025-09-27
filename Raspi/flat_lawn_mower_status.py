from dataclasses import dataclass
import struct

# Define NUM_ENCODERS (must match STM32)
NUM_ENCODERS = 2

# Flattened struct format string (must match C struct in communication.h)
# Use '<' for little-endian (common for ARM, verify STM32's endianness)
# 'i': int32_t, 'I': uint32_t, 'f': float, 'B': uint8_t, 'c': char (single byte)
# We need to explicitly handle arrays and strings.
# '30s': char[30]
# '2B': uint8_t wheel_direction[NUM_ENCODERS]
# '3f': float pos[3]
# '8?': bool bumpers[8] (Python's bool maps to 1 byte, usually)
FLAT_MOWER_STATUS_FORMAT = "<ii30s2BIIff3f3f3f3f8B4BBBBffBBIBBBB"

@dataclass
class FlatLawnMowerStatus:
    left_motor_speed: int
    right_motor_speed: int
    direction: str # Changed to str for decoded string
    wheel_direction: tuple # For 2 uint8_t
    left_encoder_count: int
    right_encoder_count: int
    speed_mps: float
    heading_deg: float
    pos_x: float
    pos_y: float
    pos_z: float
    accel_x: float
    accel_y: float
    accel_z: float
    gyro_x: float
    gyro_y: float
    gyro_z: float
    euler_roll: float
    euler_pitch: float
    euler_yaw: float
    bumpers: tuple # For 8 bool
    irda_distance: tuple # For 4 uint8_t
    rain_detected: int
    blade_motor_status: int
    blade_speed_rpm: int
    battery_voltage: float
    battery_current: float
    battery_percentage: int
    charging_status: int
    uptime_ms: int
    error_code: int
    is_manual_mode: int
    is_emergency_stop: int
    task_state: int
    edge_sensor: int

    @classmethod
    def from_bytes(cls, data: bytes):
        unpacked_data = struct.unpack(FLAT_MOWER_STATUS_FORMAT, data)
        
        dat_iter = iter(unpacked_data)

        return cls(
            left_motor_speed=next(dat_iter),
            right_motor_speed=next(dat_iter),
            direction=next(dat_iter).decode('utf-8').strip('\x00'), # Decode and strip null bytes
            wheel_direction=tuple(next(dat_iter) for _ in range(NUM_ENCODERS)), # 2 uint8_t
            left_encoder_count=next(dat_iter),
            right_encoder_count=next(dat_iter),
            speed_mps=next(dat_iter),
            heading_deg=next(dat_iter),
            pos_x=next(dat_iter),
            pos_y=next(dat_iter),
            pos_z=next(dat_iter),
            accel_x=next(dat_iter),
            accel_y=next(dat_iter),
            accel_z=next(dat_iter),
            gyro_x=next(dat_iter),
            gyro_y=next(dat_iter),
            gyro_z=next(dat_iter),
            euler_roll=next(dat_iter),
            euler_pitch=next(dat_iter),
            euler_yaw=next(dat_iter),
            bumpers=tuple(bool(next(dat_iter)) for _ in range(8)), # 8 bool
            irda_distance=tuple(next(dat_iter) for _ in range(4)), # 4 uint8_t
            rain_detected=next(dat_iter),
            blade_motor_status=next(dat_iter),
            blade_speed_rpm=next(dat_iter),
            battery_voltage=next(dat_iter),
            battery_current=next(dat_iter),
            battery_percentage=next(dat_iter),
            charging_status=next(dat_iter),
            uptime_ms=next(dat_iter),
            error_code=next(dat_iter),
            is_manual_mode=next(dat_iter),
            is_emergency_stop=next(dat_iter),
            task_state=next(dat_iter),
            edge_sensor=next(dat_iter),
        )

    def __str__(self):
        return f"""
FlatLawnMowerStatus:
  Motors: L={self.left_motor_speed}, R={self.right_motor_speed}
  Direction: '{self.direction}', Wheel_Direction: {self.wheel_direction}
  Encoders: L={self.left_encoder_count}, R={self.right_encoder_count}
  Speed: {self.speed_mps:.2f} m/s, Heading: {self.heading_deg:.2f}°
  Position: ({self.pos_x:.2f}, {self.pos_y:.2f}, {self.pos_z:.2f})
  Acceleration: ({self.accel_x:.2f}, {self.accel_y:.2f}, {self.accel_z:.2f})
  Gyro: ({self.gyro_x:.2f}, {self.gyro_y:.2f}, {self.gyro_z:.2f})
  Euler Angles: ({self.euler_roll:.2f}, {self.euler_pitch:.2f}, {self.euler_yaw:.2f})
  Bumpers: {list(self.bumpers)}
  IRDA Distance: {list(self.irda_distance)}
  Rain Detected: {bool(self.rain_detected)}, Blade Motor Status: {bool(self.blade_motor_status)}, Blade Speed: {self.blade_speed_rpm} RPM
  Battery: Voltage={self.battery_voltage:.2f}V, Current={self.battery_current:.2f}A, Pct={self.battery_percentage}%, Charging={bool(self.charging_status)}
  Uptime: {self.uptime_ms} ms, Error Code: {hex(self.error_code)}
  Mode: Manual={bool(self.is_manual_mode)}, Emergency Stop={bool(self.is_emergency_stop)}
  Task State: {self.task_state}, Edge Sensor: {self.edge_sensor}
        """
