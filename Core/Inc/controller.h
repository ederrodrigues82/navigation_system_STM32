#include <main.h>

typedef struct {
    // All pointer parameters are updated by actuators and sensor
	// === Motion control ===
    int *left_motor_speed;         // PWM duty cycle or RPM
    int *right_motor_speed;        // PWM duty cycle or RPM
    char direction[30];  // receive the direction_names[] const char from actuators.c
    uint8_t wheel_direction[NUM_ENCODERS];              // STOP, FORWARD, BACKWARD, etc.
    uint32_t *left_encoder_count;
    uint32_t *right_encoder_count;
    float *speed_mps;             // meters per second
    float *heading_deg;           // orientation from IMU or GPS

    // === Positioning ===
    float *pos[3];                  // X, Y and Z position in cm
    float *accel[3];                // X, Y and Z accelaration
    float *gyro[3];                 // X, Y and Z gyro in rad
    float *euler_angles[3];         // yaw, pitch, roll

    // === Sensors ===
    bool *bumpers[8];        // FRONT, REAR, RIGHT, LEFT
    						// FRONT_RIGHT, FRONT_LEFT
							// REAR_RIGHT, REAR_LEFT, RIGHT, LEFT
    						// 1 = hit, 0 = no collision
    uint8_t *irda_distance[4]; // FRONT, REAR, LEFT, RIGHT in cm
    uint8_t rain_detected;       // 1 = rain, pause mowing
    uint8_t blade_motor_status;  // 1 = ON, 0 = OFF
    uint8_t *blade_speed_rpm;

    // === Battery and power ===
    float battery_voltage;       // Volts
    float battery_current;       // Amperes
    uint8_t battery_percentage;  // %
    uint8_t charging_status;     // 1 = charging, 0 = discharging

    // === System status ===
    uint32_t uptime_ms;          // milliseconds since startup
    uint8_t error_code;          // error flags
    uint8_t is_manual_mode;      // 1 = manual, 0 = automatic
    uint8_t is_emergency_stop;   // 1 = stop, 0 = run
    uint8_t task_state;          // IDLE, MOWING, RETURNING_HOME, etc.

} lawn_mower_status;
