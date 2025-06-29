//Simulate the encoders signals
//Simulate the movement back and forward
//Simulate the rotate movement

#include <mov_simulator.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"

const char *direction_names[] = {
    "STOP",             // 0
    "FORWARD",          // 1
    "BACKWARD",         // 2
    "ROTATING_CLOCK",   // 3
    "ROTATING_COUNTER"  // 4
};

char status = STOP;

volatile int count_right = 0;
volatile int count_left = 0;
volatile int limit_right = 0;
volatile int limit_left = 0;

int set_movement(int direction, int count)
{
    status = direction;

    printf("Current direction: %s\r\n", direction_names[direction]);

    if (direction == STOP)
    {

    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
        return 0;
    }

    count_right = 0;
    count_left = 0;
    limit_right = count;
    limit_left = count;
    printf("Moving %d pulses forward\r\n", count);
    return 1;
}

static int update_movement(void)
{
    // Increment simulated encoder pulse counters
    count_right++;
    count_left++;

    // Stop if limit reached
    if (count_right >= limit_right || count_left >= limit_left)
    {
        status = STOP;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
        return 0;
    }

    return 1;
}

void tick(void)
{
    if (status != STOP)
    {
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // LED
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);

        // Only count on rising edge (when PC14 becomes high)
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14) == GPIO_PIN_SET)
        {
            update_movement();
        }
    }
}
