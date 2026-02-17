/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMUNICATION_TEST_H
#define __COMMUNICATION_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "controller.h"
#include "communication.h"

// Function declarations
void run_uart_test(lawn_mower_status *m_status);

extern const uint8_t uart_ping_message[];
extern const uint16_t UART_PING_MESSAGE_SIZE;

#ifdef __cplusplus
}
#endif

#endif /* __COMMUNICATION_TEST_H__ */
