#include "communication_test.h"
#include "main.h"
#include "controller.h"
#include "communication.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

// Define a simple ping message
const uint8_t uart_ping_message[] = "PING";
const uint16_t UART_PING_MESSAGE_SIZE = sizeof(uart_ping_message) - 1; // -1 to exclude null terminator

void run_uart_test(lawn_mower_status* m_status) {
    (void)m_status;  /* Unused for ping test */
    // RPi initiates: STM32 waits for request, then responds
        // LOG_INFO("Running UART test");
        // m_status->left_motor_speed = 100;
        // m_status->right_motor_speed = 120;
        // m_status->left_encoder_count = 500;
        // m_status->right_encoder_count = 520;
        // m_status->speed_mps = 0.5f;
        // m_status->heading_deg = 45.0f;
        // m_status->blade_speed_rpm = 200;
        // LOG_INFO("Initialized motors speed");

        // // Populate arrays
        // float pos[3] = {10.0f, 20.0f, 0.0f};
        // m_status->pos[0] = pos[0];
        // m_status->pos[1] = pos[1];
        // m_status->pos[2] = pos[2];
        // float accel[3] = {0.1f, 0.05f, 9.81f};
        // m_status->accel[0] = accel[0];
        // m_status->accel[1] = accel[1];
        // m_status->accel[2] = accel[2];
        // float gyro[3] = {0.01f, 0.02f, 0.03f};
        // m_status->gyro[0] = gyro[0];
        // m_status->gyro[1] = gyro[1];
        // m_status->gyro[2] = gyro[2];
        // float euler_angles[3] = {1.0f, 2.0f, 3.0f};
        // m_status->euler_angles[0] = euler_angles[0];
        // m_status->euler_angles[1] = euler_angles[1];
        // m_status->euler_angles[2] = euler_angles[2];
        // LOG_INFO("Initialized gyroscope data");
        //
        // bool bumpers[8] = {0, 1, 0, 1, 0, 1, 0, 1};
        // m_status->bumpers[0] = bumpers[0];
        // m_status->bumpers[1] = bumpers[1];
        // m_status->bumpers[2] = bumpers[2];
        // m_status->bumpers[3] = bumpers[3];
        // m_status->bumpers[4] = bumpers[4];
        // m_status->bumpers[5] = bumpers[5];
        // m_status->bumpers[6] = bumpers[6];
        // m_status->bumpers[7] = bumpers[7];
        // LOG_INFO("Initialized bumpers");
        //
        // uint8_t irda_distance[4] = {10, 20, 30, 40};
        // m_status->irda_distance[0] = irda_distance[0];
        // m_status->irda_distance[1] = irda_distance[1];
        // m_status->irda_distance[2] = irda_distance[2];
        // m_status->irda_distance[3] = irda_distance[3];
        // LOG_INFO("Initialized irda distance");
        //
        // strncpy(m_status->direction, "TEST_FORWARD", sizeof(m_status->direction) - 1);
        // m_status->direction[sizeof(m_status->direction) - 1] = '\0';
        // m_status->rain_detected = 1;
        // m_status->blade_motor_status = 1;
        // m_status->blade_speed_rpm = 200;
        // LOG_INFO("Initialized blade motor status");
        //
        // LOG_INFO("Initializing battery status");
        // m_status->battery_voltage = 12.5f;
        // m_status->battery_current = 1.2f;
        // m_status->battery_percentage = 75;
        // m_status->charging_status = 50;
        // m_status->uptime_ms = HAL_GetTick();
        // LOG_INFO("Uptime: %lu", m_status->uptime_ms);
        // m_status->error_code = 0x01; // Example error code
        // m_status->is_manual_mode = 0;
        // m_status->is_emergency_stop = 0;
        // m_status->task_state = 1; // STATE_FOLLOW_EDGE
        // m_status->edge_sensor = 2; // EDGE_SENSOR_OFF
        //
        // flat_lawn_mower_status flat_m_status;
        // serialize_lawn_mower_status(m_status, &flat_m_status);
        // LOG_INFO("Serialized lawn mower status. Size: %d bytes", sizeof(flat_lawn_mower_status));

    // RPi initiates: STM32 waits for request, then responds
    uint8_t rx_buffer[UART_PING_MESSAGE_SIZE];
    HAL_StatusTypeDef rx_status = HAL_UART_Receive(&huart2, rx_buffer, UART_PING_MESSAGE_SIZE, HAL_MAX_DELAY);

    if (rx_status == HAL_OK) {
        LOG_INFO("Received request from RPi, sending PING response.");
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_StatusTypeDef tx_status = HAL_UART_Transmit(&huart2, (uint8_t*)uart_ping_message, UART_PING_MESSAGE_SIZE, HAL_MAX_DELAY);
        if (tx_status != HAL_OK) {
            LOG_ERROR("UART response transmission failed.");
        }
    } else {
        LOG_ERROR("UART receive failed or timed out.");
    }
}
