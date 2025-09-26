/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACTUATORS_H
#define __ACTUATORS_H

#include <stdint.h>
#include <stm32f1xx_hal.h>

#define NUM_ENCODERS 2
#define ENCODER_RIGHT 0
#define ENCODER_LEFT 1
#define WHEEL_RIGHT 0
#define WHEEL_LEFT 1

const char* wheel_to_str[] = { "right wheel", "left wheel" };

typedef enum {
	FORWARD,
	BACKWARD,
	STOP,
	ROTATING_CLOCK,
	ROTATING_COUNTER
} Movement_direction;

const char* movement_directon_to_str[] = { "FORWARD", "BACKWARD", "STOP", "ROTATING_CLOCK", "ROTATING_COUNTER" };

extern const char *direction_names[];

void print_wheel_status(uint8_t encoder);
int32_t get_encoder_position(uint8_t encoder);
void measure_encoders(TIM_HandleTypeDef *htim);
void set_left_wheel(uint8_t direction, int count);
void set_right_wheel(uint8_t direction, int count);

#endif /* ACTUATORS_H */
