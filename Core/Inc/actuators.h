/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACTUATORS_H
#define __ACTUATORS_H

#include <stdint.h>
#include <stm32f1xx_hal.h>

#define FORWARD             1
#define BACKWARD            2
#define STOP                0
#define ROTATING_CLOCK      3
#define ROTATING_COUNTER    4
#define NUM_ENCODERS 2
#define ENCODER_RIGHT 0
#define ENCODER_LEFT 1

extern const char *direction_names[];

void print_wheel_status(uint8_t encoder);
int32_t get_encoder_position(uint8_t encoder);
void measure_encoders(TIM_HandleTypeDef *htim);
void set_left_wheel(uint8_t direction, uint8_t count);
void set_right_wheel(uint8_t direction, uint8_t count);

#endif /* ACTUATORS_H */
