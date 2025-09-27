#include "communication_test.h"
#include "main.h"
#include "controller.h"
#include "communication.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

void run_spi_test(lawn_mower_status* m_status) {
    // Mock/populate the lawn_mower_status with test data
    m_status->left_motor_speed = 100;
    m_status->right_motor_speed = 120;
    m_status->left_encoder_count = 500;
    m_status->right_encoder_count = 520;
    m_status->speed_mps = 0.5f;
    m_status->heading_deg = 45.0f;
    m_status->blade_speed_rpm = 200;

    // Populate arrays
    float pos[3] = {10.0f, 20.0f, 0.0f};
    m_status->pos[0] = pos[0];
    m_status->pos[1] = pos[1];
    m_status->pos[2] = pos[2];
    float accel[3] = {0.1f, 0.05f, 9.81f};
    m_status->accel[0] = accel[0];
    m_status->accel[1] = accel[1];
    m_status->accel[2] = accel[2];
    float gyro[3] = {0.01f, 0.02f, 0.03f};
    m_status->gyro[0] = gyro[0];
    m_status->gyro[1] = gyro[1];
    m_status->gyro[2] = gyro[2];
    float euler_angles[3] = {1.0f, 2.0f, 3.0f};
    m_status->euler_angles[0] = euler_angles[0];
    m_status->euler_angles[1] = euler_angles[1];
    m_status->euler_angles[2] = euler_angles[2];

    bool bumpers[8] = {0, 1, 0, 1, 0, 1, 0, 1};
    
    m_status->bumpers[0] = bumpers[0];
    m_status->bumpers[1] = bumpers[1];
    m_status->bumpers[2] = bumpers[2];
    m_status->bumpers[3] = bumpers[3];
    m_status->bumpers[4] = bumpers[4];
    m_status->bumpers[5] = bumpers[5];
    m_status->bumpers[6] = bumpers[6];
    m_status->bumpers[7] = bumpers[7];
    
    uint8_t irda_distance[4] = {10, 20, 30, 40};
    m_status->irda_distance[0] = irda_distance[0];
    m_status->irda_distance[1] = irda_distance[1];
    m_status->irda_distance[2] = irda_distance[2];
    m_status->irda_distance[3] = irda_distance[3];

    strncpy(m_status->direction, "TEST_FORWARD", sizeof(m_status->direction) - 1);
    m_status->direction[sizeof(m_status->direction) - 1] = '\0';
    m_status->rain_detected = 1;
    m_status->blade_motor_status = 1;
    m_status->blade_speed_rpm = 200;
    
    m_status->battery_voltage = 12.5f;
    m_status->battery_current = 1.2f;
    m_status->battery_percentage = 75;
    m_status->charging_status = 50;
    m_status->uptime_ms = HAL_GetTick();
    m_status->error_code = 0x01; // Example error code
    m_status->is_manual_mode = 0;
    m_status->is_emergency_stop = 0;
    m_status->task_state = 1; // STATE_FOLLOW_EDGE
    m_status->edge_sensor = 2; // EDGE_SENSOR_OFF

    flat_lawn_mower_status flat_m_status;
    serialize_lawn_mower_status(m_status, &flat_m_status);

    uint8_t rx_buffer[sizeof(flat_lawn_mower_status)];
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&flat_m_status, rx_buffer, sizeof(flat_m_status), HAL_MAX_DELAY);

    // In a real scenario, you might want to process rx_buffer here to get data from Raspi
}
