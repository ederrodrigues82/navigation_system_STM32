#include "communication_test.h"
#include "main.h"
#include "controller.h"
#include "communication.h"
#include "actuators.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

// Define a simple ping message
const uint8_t uart_ping_message[] = "PING";
const uint16_t UART_PING_MESSAGE_SIZE = sizeof(uart_ping_message) - 1; // -1 to exclude null terminator

void run_uart_test(lawn_mower_status* m_status) {
    // RPi initiates: STM32 waits for 1-byte command, then responds
    uint8_t cmd;
    HAL_StatusTypeDef rx_status = HAL_UART_Receive(&huart2, &cmd, 1, HAL_MAX_DELAY);

    if (rx_status != HAL_OK) {
        LOG_ERROR("UART receive failed or timed out.");
        return;
    }

    if (cmd == CMD_PING) {
        LOG_INFO("Received PING, sending response.");
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_StatusTypeDef tx_status = HAL_UART_Transmit(&huart2, (uint8_t*)uart_ping_message, UART_PING_MESSAGE_SIZE, HAL_MAX_DELAY);
        if (tx_status != HAL_OK) {
            LOG_ERROR("UART PING transmission failed.");
        }
    } else if (cmd == CMD_STATUS) {
        get_wheel_status(m_status->wheel_status);
        flat_lawn_mower_status flat_m_status;
        serialize_lawn_mower_status(m_status, &flat_m_status);
        size_t payload_size = sizeof(flat_lawn_mower_status);
        HAL_StatusTypeDef tx_status = HAL_UART_Transmit(&huart2, (uint8_t*)&flat_m_status, payload_size, HAL_MAX_DELAY);
        if (tx_status == HAL_OK) {
            LOG_INFO("Sent lawn mower status (%u bytes).", (unsigned)payload_size);
        } else {
            LOG_ERROR("UART status transmission failed.");
        }
    } else if (cmd == CMD_MOVE) {
        uint8_t move_buf[6];  /* wheel(1) + direction(1) + distance(4) */
        HAL_StatusTypeDef move_rx = HAL_UART_Receive(&huart2, move_buf, sizeof(move_buf), HAL_MAX_DELAY);
        if (move_rx == HAL_OK) {
            uint8_t wheel = move_buf[0];
            uint8_t direction = move_buf[1];
            int32_t distance;
            memcpy(&distance, &move_buf[2], sizeof(distance));
            if (wheel == WHEEL_RIGHT) {
                set_right_wheel(direction, (int)distance);
            } else if (wheel == WHEEL_LEFT) {
                set_left_wheel(direction, (int)distance);
            } else if (wheel == 2) { /* WHEEL_BOTH */
                set_right_wheel(direction, (int)distance);
                set_left_wheel(direction, (int)distance);
            } else {
                LOG_WARNING("Invalid wheel: %u", wheel);
            }
        } else {
            LOG_ERROR("CMD_MOVE receive failed.");
        }
    } else {
        LOG_WARNING("Unknown command: 0x%02X", cmd);
    }
}
