//Manage the actuators and sensors:
//Motor and encoder on the right wheel
//Motor and encoder on the left wheel

#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <actuators.h>
#include <mov_simulator.h>
#include <stm32f1xx_hal.h>

#define SIM_MOVEMENT 1 //Movement simulation

int32_t encoder_position[NUM_ENCODERS] = {0};
uint8_t last_b_state[NUM_ENCODERS] = {0};  // For direction detection
uint8_t wheel_direction[NUM_ENCODERS] = {STOP, STOP};

// Pulse encoders targets
volatile uint32_t pulse_count[NUM_ENCODERS] = {0};
volatile uint32_t target_count[NUM_ENCODERS] = {0};

const char *direction_names[] = {
    "STOP",             // 0
    "FORWARD",          // 1
    "BACKWARD",         // 2
    "ROTATING_CLOCK",   // 3
    "ROTATING_COUNTER"  // 4
};

void set_target_count(uint8_t channel, uint32_t target);

void set_right_wheel(uint8_t direction, uint8_t count) {
	//TODO implement the hardware pin for motor actuation
	wheel_direction[ENCODER_RIGHT] = direction;
	set_target_count(ENCODER_RIGHT, count); //Set number pulses for the movement
	if (SIM_MOVEMENT) {
		set_simulate_movement(direction, count);
	}
	printf("Moving to direction: %s, pulses: %d\r\n", direction_names[direction], count);

	if (direction == STOP)
	{
//      TODO set pin to 0
		return;
	}
//	TODO set pin to 1
	printf("Moving %d pulses forward\r\n", count);
	return;
}

void set_left_wheel(uint8_t direction, uint8_t count) {
	//TODO implement the hardware pin for motor actuation
	wheel_direction[ENCODER_LEFT] = direction;
	set_target_count(ENCODER_LEFT, count); //Set number pulses for the movement
	if (SIM_MOVEMENT) {
		set_simulate_movement(direction, count);
	}
	printf("Moving to direction: %s, pulses: %d\r\n", direction_names[direction], count);

	if (direction == STOP)
	{
//      TODO set pin to 0
		return;
	}
//	TODO set pin to 1

	return;
}

void measure_encoders(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        uint8_t encoder = 0xFF;
        GPIO_TypeDef* b_port = NULL;
        uint16_t b_pin = 0;

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            encoder = ENCODER_RIGHT;
            b_port = GPIOA; // TIM2_CH2 → GPIOA (por exemplo)
            b_pin  = GPIO_PIN_4; // Encoder right pin B
        }
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
            encoder = ENCODER_LEFT;
            b_port = GPIOA; // TIM2_CH4 → GPIOA (por exemplo)
            b_pin  = GPIO_PIN_5; // Encoder left pin B
        }

        if (encoder < NUM_ENCODERS) {
            // capture signal B
            uint8_t b_state = HAL_GPIO_ReadPin(b_port, b_pin);

            // determine direction based on signal B
            if (b_state == 0)
                encoder_position[encoder]++;
            else
                encoder_position[encoder]--;

            if (abs(encoder_position[encoder]) >= target_count[encoder]) {
                // Stop respective channel A capture
                HAL_TIM_IC_Stop_IT(htim, (encoder == 0) ? TIM_CHANNEL_1 : TIM_CHANNEL_3);
                // Stop the respective wheel
                switch (encoder) {
                    case 0:
                        set_right_wheel(STOP, 0);
                        break;
                    case 1:
                        set_left_wheel(STOP, 0);
                        break;
                }
            }
        }
    }
}

//Function uset to set the target to count external events (encoders)
void set_target_count(uint8_t channel, uint32_t target) {
    if (channel < NUM_CHANNELS) {
        target_count[channel] = target;
        pulse_count[channel] = 0;
        start_tim2(channel);
    }
}

int32_t get_encoder_position(uint8_t encoder) {
    if (encoder < NUM_ENCODERS)
        return encoder_position[encoder];
    return 0;
}

void print_wheel_status(uint8_t encoder) {
    if (encoder >= NUM_ENCODERS) {
        printf("Invalid encoder index: %d\r\n", encoder);
        return;
    }

    printf("\r\nWheel %s:\r\n", (encoder == ENCODER_RIGHT) ? "RIGHT" : "LEFT");
    printf("  Direction     : %s\r\n", direction_names[wheel_direction[encoder]]);
    printf("  Target Count  : %ld\r\n", target_count[encoder]);
    printf("  Position      : %ld\r\n", encoder_position[encoder]);
}
