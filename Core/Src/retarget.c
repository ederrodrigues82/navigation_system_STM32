#include "stm32f1xx_hal.h"
#include <unistd.h> // for write()

extern UART_HandleTypeDef huart1;

int _write(int file, char *data, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
    return len;
}
