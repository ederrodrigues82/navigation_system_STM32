#include "communication.h"
#include <string.h>

// Assuming NUM_ENCODERS is defined in controller.h or a common header
// For now, it's defined in communication.h

void serialize_lawn_mower_status(const lawn_mower_status *src, flat_lawn_mower_status *dest) {
    dest->left_motor_speed = src->left_motor_speed;
    dest->right_motor_speed = src->right_motor_speed;
    strncpy(dest->direction, src->direction, sizeof(dest->direction) - 1);
    dest->direction[sizeof(dest->direction) - 1] = '\0'; // Ensure null-termination

    for (int i = 0; i < NUM_ENCODERS; i++) {
        dest->wheel_direction[i] = src->wheel_direction[i];
    }
    dest->left_encoder_count = src->left_encoder_count;
    dest->right_encoder_count = src->right_encoder_count;
    dest->speed_mps = src->speed_mps;
    dest->heading_deg = src->heading_deg;

    for (int i = 0; i < 3; i++) {
        dest->pos[i] = src->pos[i];
        dest->accel[i] = src->accel[i];
        dest->gyro[i] = src->gyro[i];
        dest->euler_angles[i] = src->euler_angles[i];
    }

    for (int i = 0; i < 8; i++) {
        dest->bumpers[i] = src->bumpers[i];
    }
    for (int i = 0; i < 4; i++) {
        dest->irda_distance[i] = src->irda_distance[i];
    }
    dest->rain_detected = src->rain_detected;
    dest->blade_motor_status = src->blade_motor_status;
    dest->blade_speed_rpm = src->blade_speed_rpm;

    dest->battery_voltage = src->battery_voltage;
    dest->battery_current = src->battery_current;
    dest->battery_percentage = src->battery_percentage;
    dest->charging_status = src->charging_status;

    dest->uptime_ms = src->uptime_ms;
    dest->error_code = src->error_code;
    dest->is_manual_mode = src->is_manual_mode;
    dest->is_emergency_stop = src->is_emergency_stop;
    dest->task_state = src->task_state;
    // movement_FIFO robot_mov; // Skipping for now, needs careful consideration
    dest->edge_sensor = src->edge_sensor;
}

// Placeholder for deserialize function, primarily for Raspi side logic
void deserialize_lawn_mower_status(const flat_lawn_mower_status *src, lawn_mower_status *dest) {
    // This function would be implemented on the Raspberry Pi side in Python
    // For STM32, it might not be strictly necessary unless bidirectional communication is needed.
    // We'll leave it as a placeholder for now.
}
