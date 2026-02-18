/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMUNICATION_TEST_H
#define __COMMUNICATION_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "controller.h"
#include "communication.h"

// UART command bytes (RPi sends 1 byte, STM32 responds accordingly)
#define CMD_PING   0x01
#define CMD_STATUS 0x02

// Function declarations
void run_uart_test(lawn_mower_status *m_status);

extern const uint8_t uart_ping_message[];
extern const uint16_t UART_PING_MESSAGE_SIZE;

#ifdef __cplusplus
}
#endif

#endif /* __COMMUNICATION_TEST_H__ */
