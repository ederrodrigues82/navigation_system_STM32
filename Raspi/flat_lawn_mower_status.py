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
        # For better readability, consider formatting this output more cleanly.
        # This is a basic representation.
        return f"FlatLawnMowerStatus({{
    left_motor_speed: {self.left_motor_speed},
    right_motor_speed: {self.right_motor_speed},
    direction: '{self.direction}',
    wheel_direction: {self.wheel_direction},
    left_encoder_count: {self.left_encoder_count},
    right_encoder_count: {self.right_encoder_count},
    speed_mps: {self.speed_mps},
    heading_deg: {self.heading_deg},
    pos: [{self.pos_x}, {self.pos_y}, {self.pos_z}],
    accel: [{self.accel_x}, {self.accel_y}, {self.accel_z}],
    gyro: [{self.gyro_x}, {self.gyro_y}, {self.gyro_z}],
    euler_angles: [{self.euler_roll}, {self.euler_pitch}, {self.euler_yaw}],
    bumpers: {list(self.bumpers)},
    irda_distance: {list(self.irda_distance)},
    rain_detected: {self.rain_detected},
    blade_motor_status: {self.blade_motor_status},
    blade_speed_rpm: {self.blade_speed_rpm},
    battery_voltage: {self.battery_voltage},
    battery_current: {self.battery_current},
    battery_percentage: {self.battery_percentage},
    charging_status: {self.charging_status},
    uptime_ms: {self.uptime_ms},
    error_code: {self.error_code},
    is_manual_mode: {self.is_manual_mode},
    is_emergency_stop: {self.is_emergency_stop},
    task_state: {self.task_state},
    edge_sensor: {self.edge_sensor}
}})
