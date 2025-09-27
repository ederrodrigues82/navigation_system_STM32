//Manage the actuators and sensors:
//Motor and encoder on the right wheel
//Motor and encoder on the left wheel

#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <actuators.h>
#include <controller.h>
#include <mov_simulator.h>
#include <stm32f1xx_hal.h>

const char* wheel_to_str[] = { "right wheel", "left wheel" };
const char* movement_directon_to_str[] = { "FORWARD", "BACKWARD", "STOP", "ROTATING_CLOCK", "ROTATING_COUNTER" };
const char *direction_names[] = {"STOP", "FORWARD", "BACKWARD", "ROTATING CLOCK", "ROTATING COUNTER"};

#define SIM_MOVEMENT 1 //Movement simulation

static int32_t encoder_position[NUM_ENCODERS] = {0};
//static uint8_t last_b_state[NUM_ENCODERS] = {0};  // For direction detection
static uint8_t wheel_direction[NUM_ENCODERS] = {STOP, STOP};
static int32_t right_motor_speed = 0; //TODO need implementation
static int32_t left_motor_speed = 0; //TODO need implementation
static float speed_mps = 0.0f; //TODO need implementation
static float heading_deg = 0; //TODO need implementation, link to BNO08X
static char direction_str[30]; // Changed to array type

// Pulse encoders targets
static volatile uint32_t pulse_count[NUM_ENCODERS] = {0};
static volatile uint32_t target_count[NUM_ENCODERS] = {0};

void set_target_count(uint8_t channel, uint32_t target);

int motion_control_init(lawn_mower_status* law_mower) {
	law_mower->left_motor_speed = left_motor_speed;         // PWM duty cycle or RPM
	law_mower->right_motor_speed = right_motor_speed;        // PWM duty cycle or RPM
	// Removed: law_mower->wheel_direction[NUM_ENCODERS] = wheel_direction[0]; // This was an out-of-bounds access and should be handled by iterating or memcopy if needed.
	law_mower->left_encoder_count = pulse_count[ENCODER_RIGHT];
	law_mower->right_encoder_count = pulse_count[ENCODER_LEFT];
	law_mower->speed_mps = speed_mps;             // meters per second
	law_mower->heading_deg = heading_deg;         // orientation from IMU or GPS
	strncpy(law_mower->direction, direction_str, sizeof(law_mower->direction) - 1);
	law_mower->direction[sizeof(law_mower->direction) - 1] = '\0';
	return 0;
}


void set_wheel(uint8_t wheel, uint8_t direction, int count) {
	//TODO implement the hardware pin for motor actuation
	if (wheel == WHEEL_RIGHT){
		wheel_direction[ENCODER_RIGHT] = direction;
		set_target_count(ENCODER_RIGHT, count); //Set number pulses for the movement
	} else {
		wheel_direction[ENCODER_LEFT] = direction;
		set_target_count(ENCODER_LEFT, count); //Set number pulses for the movement
	}

	if (SIM_MOVEMENT) {
		set_simulate_movement(direction, count);
	}
	LOG_INFO("Moving %s to direction: %s, pulses: %d", wheel_to_str[wheel], movement_directon_to_str[direction], count);

    if (direction == FORWARD || direction == ROTATING_CLOCK) {
    	//set on right wheel motor
    	//set the direction of motor to clock
    }

    if (direction == BACKWARD || direction == ROTATING_COUNTER) {
		//set on right wheel motor
		//set the direction of motor to counter clock
    }
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
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
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
                HAL_TIM_IC_Stop_IT(htim, (encoder == 0) ? TIM_CHANNEL_1 : TIM_CHANNEL_2);
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
        LOG_ERROR("Invalid encoder index: %d", encoder);
        return;
    }

    LOG_INFO("\nWheel %s:", (encoder == ENCODER_RIGHT) ? "RIGHT" : "LEFT");
    LOG_INFO("  Direction     : %s", direction_names[wheel_direction[encoder]]);
    LOG_INFO("  Target Count  : %ld", target_count[encoder]);
    LOG_INFO("  Position      : %ld", encoder_position[encoder]);
}

void turn_45 (Direction direction) {

}

void set_left_wheel(uint8_t direction, int count) {
    set_wheel(WHEEL_LEFT, direction, count);
}

void set_right_wheel(uint8_t direction, int count) {
    set_wheel(WHEEL_RIGHT, direction, count);
}
