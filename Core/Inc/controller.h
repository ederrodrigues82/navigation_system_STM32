#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include <main.h>
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

typedef enum {
    STATE_ALIGN_TO_EDGE,
    STATE_FOLLOW_EDGE,
    STATE_CLOSED_AREA_DETECTED,
    STATE_ZIGZAG,
    STATE_AVOID_OBSTACLE
} MowerState;

typedef enum {
    EDGE_SENSOR_OFF,
	EDGE_SENSOR_TOUCH,
	EDGE_SENSOR_HARD_TOUCH,
	EDGE_SENSOR_CUT,
	EDGE_SENSOR_BLOCKED
} Edge_sensor;

typedef enum {
	SOFT_FORWARD = 15,
	SOFT_BACKWARD = -15,
	SOFT_CLOCK = 15,
	SOFT_COUNTER = -15,
	TURN_CLOCK = 50,
	TURN_COUNTER = -50
} Movements_steps;

#define FIFO_SIZE 10  // Define the buffer size

typedef struct {
    uint8_t direction;
    int count;
} movement;

typedef struct {
    movement buffer[FIFO_SIZE];
    int head;  // Where to read from
    int tail;  // Where to write to
    int count; // Number of items currently in buffer
} Movement_FIFO;

typedef struct {
    // All pointer parameters are updated by actuators and sensor
	// === Motion control ===
	int32_t left_motor_speed;         // PWM duty cycle or RPM
	int32_t right_motor_speed;        // PWM duty cycle or RPM
    char direction[30];  // receive the direction_names[] const char from actuators.c
    uint8_t wheel_direction[NUM_ENCODERS];              // STOP, FORWARD, BACKWARD, etc.
    uint8_t wheel_status[NUM_ENCODERS];                 // READY, ERROR, MOVING
    uint32_t left_encoder_count;
    uint32_t right_encoder_count;
    float speed_mps;             // meters per second
    float heading_deg;           // orientation from IMU or GPS

    // === Positioning ===
    float pos[3];                  // X, Y and Z position in cm
    float accel[3];                // X, Y and Z accelaration
    float gyro[3];                 // X, Y and Z gyro in rad
    float euler_angles[3];         // yaw, pitch, roll

    // === Sensors ===
    bool bumpers[8];        // FRONT, REAR, RIGHT, LEFT
    						// FRONT_RIGHT, FRONT_LEFT
							// REAR_RIGHT, REAR_LEFT, RIGHT, LEFT
    						// 1 = hit, 0 = no collision
    uint8_t irda_distance[4]; // FRONT, REAR, LEFT, RIGHT in cm
    uint8_t rain_detected;       // 1 = rain, pause mowing
    uint8_t blade_motor_status;  // 1 = ON, 0 = OFF
    uint8_t blade_speed_rpm;

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
    Movement_FIFO robot_mov;	 // FIFO with robot movements
    Edge_sensor edge_sensor;     // Rotator cut edge sensor
    MowerState mower_state;


} lawn_mower_status;

// Main control loop function
void mower_main_loop(lawn_mower_status *status);
void move_forward(int count);

#endif // __CONTROLLER_H
