#include "communication_test.h"
#include "main.h"
#include "controller.h"
#include "communication.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

// Define a simple ping message
const uint8_t spi_ping_message[] = "PING";
const uint16_t SPI_PING_MESSAGE_SIZE = sizeof(spi_ping_message) - 1; // -1 to exclude null terminator

// Function to send a simple SPI ping message
// void send_spi_ping(void)
// {
//     LOG_INFO("Attempting to send SPI ping...");
//     HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, (uint8_t*)spi_ping_message, SPI_PING_MESSAGE_SIZE, 100);
//     if (status == HAL_OK)
//     {
//         LOG_INFO("SPI Ping sent successfully!");
//     }
//     else if (status == HAL_TIMEOUT)
//     {
//         LOG_ERROR("SPI Ping transmission timed out!");
//     }
//     else
//     {
//         LOG_ERROR("SPI Ping transmission failed with status: %d", status);
//     }
// }


void run_spi_test(lawn_mower_status* m_status) {
    LOG_INFO("run_spi_test called. Current Tick: %lu", HAL_GetTick()); // Debug log
    // Removed the 2-second delay for debugging purposes
    // if (HAL_GetTick() - last_run_time >= 2000) { // Execute every 2 seconds
    static uint32_t last_run_time = HAL_GetTick();

        LOG_INFO("STM32 Ping! Uptime: %lu ms", HAL_GetTick());
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the onboard LED (PC13)

        // Mock/populate the lawn_mower_status with test data
        // LOG_INFO("Running SPI test");
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

        uint8_t rx_buffer[SPI_PING_MESSAGE_SIZE];
        LOG_INFO("Attempting SPI TransmitReceive (PING)...");
        // Removed timeout for debugging purposes. Slave will now wait indefinitely.
        HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)spi_ping_message, rx_buffer, SPI_PING_MESSAGE_SIZE, HAL_MAX_DELAY);

        if (spi_status == HAL_OK) {
            LOG_INFO("Transmitted PING and received response.");
            // In a real scenario, you might want to process rx_buffer here to get data from Raspi
            // For now, we'll just log the first few bytes of the response
            if (SPI_PING_MESSAGE_SIZE > 0) {
                LOG_INFO("Received SPI response (first byte): 0x%02X", rx_buffer[0]);
            }
        } else if (spi_status == HAL_TIMEOUT) {
            LOG_ERROR("SPI TransmitReceive (PING) timed out!");
        } else {
            LOG_ERROR("SPI TransmitReceive (PING) failed with error code: %d", spi_status);
        }
        HAL_Delay(10); // Small delay to allow the RPi to process if it did respond
    // }
}
