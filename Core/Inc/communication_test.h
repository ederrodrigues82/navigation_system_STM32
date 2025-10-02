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
void run_spi_test(lawn_mower_status *m_status);
void send_spi_ping(void);

extern const uint8_t spi_ping_message[];
extern const uint16_t SPI_PING_MESSAGE_SIZE;

#ifdef __cplusplus
}
#endif

#endif /* __COMMUNICATION_TEST_H__ */
