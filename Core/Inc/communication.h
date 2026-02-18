#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "controller.h" // Required for lawn_mower_status, MowerState, Edge_sensor
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

// Define NUM_ENCODERS if not already defined (or include actuators.h if appropriate)
// #ifndef NUM_ENCODERS
// #define NUM_ENCODERS 2
// #endif

// Flattened version of lawn_mower_status for transmission
typedef struct __attribute__((__packed__)) {
    int32_t left_motor_speed;
    int32_t right_motor_speed;
    char direction[30];
    uint8_t wheel_direction[NUM_ENCODERS];
    uint8_t wheel_status[NUM_ENCODERS];
    uint32_t left_encoder_count;
    uint32_t right_encoder_count;
    float speed_mps;
    float heading_deg;

    float pos[3];
    float accel[3];
    float gyro[3];
    float euler_angles[3];

    bool bumpers[8];
    uint8_t irda_distance[4];
    uint8_t rain_detected;
    uint8_t blade_motor_status;
    uint8_t blade_speed_rpm;

    float battery_voltage;
    float battery_current;
    uint8_t battery_percentage;
    uint8_t charging_status;

    uint32_t uptime_ms;
    uint8_t error_code;
    uint8_t is_manual_mode;
    uint8_t is_emergency_stop;
    uint8_t task_state; // MowerState enum value
    // movement_FIFO robot_mov; // We need to decide how to handle this complex struct
    uint8_t edge_sensor; // Edge_sensor enum value
} flat_lawn_mower_status;

void serialize_lawn_mower_status(const lawn_mower_status *src, flat_lawn_mower_status *dest);
void deserialize_lawn_mower_status(const flat_lawn_mower_status *src, lawn_mower_status *dest); // For Raspi side, mostly

#endif // __COMMUNICATION_H
