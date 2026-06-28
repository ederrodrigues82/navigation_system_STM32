#include "communication_test.h"
#include "main.h"
#include "controller.h"
#include "communication.h"
#include "actuators.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

const uint8_t uart_ping_message[] = "PING";
const uint16_t UART_PING_MESSAGE_SIZE = sizeof(uart_ping_message) - 1;

void run_uart_test(lawn_mower_status* m_status) {
    uint8_t cmd;
    HAL_StatusTypeDef rx_status = HAL_UART_Receive(&huart2, &cmd, 1, HAL_MAX_DELAY);

    if (rx_status != HAL_OK) {
        return;
    }

    if (cmd == CMD_PING) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_ping_message, UART_PING_MESSAGE_SIZE, HAL_MAX_DELAY);
    } else if (cmd == CMD_STATUS) {
        motion_control_init(m_status);
        flat_lawn_mower_status flat_m_status;
        serialize_lawn_mower_status(m_status, &flat_m_status);
        size_t payload_size = sizeof(flat_lawn_mower_status);
        HAL_UART_Transmit(&huart2, (uint8_t*)&flat_m_status, payload_size, HAL_MAX_DELAY);
    } else if (cmd == CMD_MOVE) {
        uint8_t move_buf[6];
        if (HAL_UART_Receive(&huart2, move_buf, sizeof(move_buf), HAL_MAX_DELAY) == HAL_OK) {
            uint8_t wheel = move_buf[0];
            uint8_t direction = move_buf[1];
            int32_t distance;
            memcpy(&distance, &move_buf[2], sizeof(distance));
            if (wheel == WHEEL_RIGHT) {
                set_right_wheel(direction, (int)distance);
            } else if (wheel == WHEEL_LEFT) {
                set_left_wheel(direction, (int)distance);
            } else if (wheel == 2) {
                set_right_wheel(direction, (int)distance);
                set_left_wheel(direction, (int)distance);
            }
        }
    } else if (cmd == CMD_SET_EMULATE_WHEEL) {
        uint8_t enable;
        if (HAL_UART_Receive(&huart2, &enable, 1, HAL_MAX_DELAY) == HAL_OK) {
            set_emulate_wheel(enable ? 1 : 0);
            uint8_t ack = get_emulate_wheel();
            HAL_UART_Transmit(&huart2, &ack, 1, HAL_MAX_DELAY);
        }
    }
}
