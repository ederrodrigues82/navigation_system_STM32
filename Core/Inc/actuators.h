/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACTUATORS_H
#define __ACTUATORS_H

#include <stdint.h>
#include <stm32f1xx_hal.h>
#include <main.h>
#include <controller.h>
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

#define ENCODER_RIGHT 0
#define ENCODER_LEFT 1
#define WHEEL_RIGHT 0
#define WHEEL_LEFT 1

extern const char* wheel_to_str[];

typedef enum {
	FORWARD,
	BACKWARD,
	STOP,
	ROTATING_CLOCK,
	ROTATING_COUNTER
} Movement_direction;

extern const char* movement_directon_to_str[];

extern const char *direction_names[];

int motion_control_init(lawn_mower_status* law_mower);
void set_wheel(uint8_t wheel, uint8_t direction, int count);
void set_target_count(uint8_t channel, uint32_t target);
void turn_45 (Direction direction);
void print_wheel_status(uint8_t encoder);
int32_t get_encoder_position(uint8_t encoder);
void measure_encoders(TIM_HandleTypeDef *htim);
void set_left_wheel(uint8_t direction, int count);
void set_right_wheel(uint8_t direction, int count);

#endif /* ACTUATORS_H */
