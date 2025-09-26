#include "communication_test.h"
#include "main.h"
#include "controller.h"
#include "communication.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

void run_spi_test(lawn_mower_status* m_status) {
    // Mock/populate the lawn_mower_status with test data
    m_status->left_motor_speed = &(int32_t){100}; // Example mock value
    m_status->right_motor_speed = &(int32_t){120}; // Example mock value
    m_status->left_encoder_count = &(uint32_t){500};
    m_status->right_encoder_count = &(uint32_t){520};
    m_status->speed_mps = &(float){0.5f};
    m_status->heading_deg = &(float){45.0f};
    m_status->blade_speed_rpm = &(uint8_t){200};

    // Populate arrays
    m_status->pos[0] = &(float){10.0f}; m_status->pos[1] = &(float){20.0f}; m_status->pos[2] = &(float){0.0f};
    m_status->accel[0] = &(float){0.1f}; m_status->accel[1] = &(float){0.05f}; m_status->accel[2] = &(float){9.81f};
    m_status->gyro[0] = &(float){0.01f}; m_status->gyro[1] = &(float){0.02f}; m_status->gyro[2] = &(float){0.03f};
    m_status->euler_angles[0] = &(float){1.0f}; m_status->euler_angles[1] = &(float){2.0f}; m_status->euler_angles[2] = &(float){3.0f};

    for(int i = 0; i < 8; i++) m_status->bumpers[i] = &(bool){(i % 2 == 0)};
    for(int i = 0; i < 4; i++) m_status->irda_distance[i] = &(uint8_t){(i * 10)};

    strncpy(m_status->direction, "TEST_FORWARD", sizeof(m_status->direction) - 1);
    m_status->direction[sizeof(m_status->direction) - 1] = '\0';
    m_status->rain_detected = 1;
    m_status->blade_motor_status = 1;
    m_status->battery_voltage = &(float){12.5f};
    m_status->battery_current = &(float){1.2f};
    m_status->battery_percentage = &(uint8_t){75};
    m_status->charging_status = 0;
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
