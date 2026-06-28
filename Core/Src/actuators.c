//Manage the actuators and sensors:
//Motor and encoder on the right wheel
//Motor and encoder on the left wheel

#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <actuators.h>
#include <controller.h>
#include <stm32f1xx_hal.h>

const char* wheel_to_str[] = { "right wheel", "left wheel" };
const char* movement_directon_to_str[] = { "FORWARD", "BACKWARD", "STOP", "ROTATING_CLOCK", "ROTATING_COUNTER" };
const char *direction_names[] = {"STOP", "FORWARD", "BACKWARD", "ROTATING CLOCK", "ROTATING COUNTER"};

static int32_t encoder_position[NUM_ENCODERS] = {0};
static uint8_t wheel_direction[NUM_ENCODERS] = {STOP, STOP};
static uint8_t wheel_status[NUM_ENCODERS] = {WHEEL_READY, WHEEL_READY};
static int32_t right_motor_speed = 0; //TODO need implementation
static int32_t left_motor_speed = 0; //TODO need implementation
static float speed_mps = 0.0f; //TODO need implementation
static float heading_deg = 0; //TODO need implementation, link to BNO08X
static char direction_str[30]; // Changed to array type
static uint8_t emulate_wheel = 0;
static uint8_t motor_active[NUM_ENCODERS] = {0};

// Pulse encoders targets
static volatile uint32_t pulse_count[NUM_ENCODERS] = {0};
static volatile uint32_t target_count[NUM_ENCODERS] = {0};

void set_target_count(uint8_t channel, uint32_t target);
static void check_encoder_target(uint8_t encoder);
static int is_motor_on(uint8_t encoder);
static void stop_motor_only(uint8_t encoder);

void set_emulate_wheel(uint8_t enable)
{
	emulate_wheel = enable ? 1 : 0;
}

uint8_t get_emulate_wheel(void)
{
	return emulate_wheel;
}

int motion_control_init(lawn_mower_status* law_mower) {
	law_mower->left_motor_speed = left_motor_speed;         // PWM duty cycle or RPM
	law_mower->right_motor_speed = right_motor_speed;        // PWM duty cycle or RPM
	for (int i = 0; i < NUM_ENCODERS; i++) {
		law_mower->wheel_direction[i] = wheel_direction[i];
		law_mower->wheel_status[i] = wheel_status[i];
	}
	law_mower->left_encoder_count = pulse_count[ENCODER_RIGHT];
	law_mower->right_encoder_count = pulse_count[ENCODER_LEFT];
	law_mower->speed_mps = speed_mps;             // meters per second
	law_mower->heading_deg = heading_deg;         // orientation from IMU or GPS
	strncpy(law_mower->direction, direction_str, sizeof(law_mower->direction) - 1);
	law_mower->direction[sizeof(law_mower->direction) - 1] = '\0';
	return 0;
}


void set_wheel(uint8_t wheel, uint8_t direction, int count) {
	if (direction == STOP || count == 0) {
		wheel_status[wheel] = WHEEL_READY;
	} else {
		wheel_status[wheel] = WHEEL_MOVING;
	}

	if (wheel == WHEEL_RIGHT){
		wheel_direction[ENCODER_RIGHT] = direction;
		set_target_count(ENCODER_RIGHT, count);
		/* Right wheel motor: PB10=IN1, PB11=IN2 */
		if (direction == STOP || count == 0) {
			HAL_GPIO_WritePin(RIGHT_MOTOR_IN1_GPIO_Port, RIGHT_MOTOR_IN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RIGHT_MOTOR_IN2_GPIO_Port, RIGHT_MOTOR_IN2_Pin, GPIO_PIN_RESET);
			motor_active[ENCODER_RIGHT] = 0;
		} else if (direction == FORWARD || direction == ROTATING_CLOCK) {
			HAL_GPIO_WritePin(RIGHT_MOTOR_IN1_GPIO_Port, RIGHT_MOTOR_IN1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(RIGHT_MOTOR_IN2_GPIO_Port, RIGHT_MOTOR_IN2_Pin, GPIO_PIN_RESET);
			motor_active[ENCODER_RIGHT] = 1;
		} else if (direction == BACKWARD || direction == ROTATING_COUNTER) {
			HAL_GPIO_WritePin(RIGHT_MOTOR_IN1_GPIO_Port, RIGHT_MOTOR_IN1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RIGHT_MOTOR_IN2_GPIO_Port, RIGHT_MOTOR_IN2_Pin, GPIO_PIN_SET);
			motor_active[ENCODER_RIGHT] = 1;
		}
	} else {
		wheel_direction[ENCODER_LEFT] = direction;
		set_target_count(ENCODER_LEFT, count);
		/* Left wheel motor: not yet implemented */
		motor_active[ENCODER_LEFT] = (direction != STOP && count != 0) ? 1 : 0;
	}

	return;
}

static void stop_motor_only(uint8_t encoder)
{
	wheel_status[encoder] = WHEEL_READY;
	if (encoder == ENCODER_RIGHT) {
		HAL_GPIO_WritePin(RIGHT_MOTOR_IN1_GPIO_Port, RIGHT_MOTOR_IN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RIGHT_MOTOR_IN2_GPIO_Port, RIGHT_MOTOR_IN2_Pin, GPIO_PIN_RESET);
		motor_active[ENCODER_RIGHT] = 0;
	} else if (encoder == ENCODER_LEFT) {
		motor_active[ENCODER_LEFT] = 0;
	}
}

static void check_encoder_target(uint8_t encoder)
{
	if (target_count[encoder] > 0 && abs(encoder_position[encoder]) >= (int32_t)target_count[encoder]) {
		stop_motor_only(encoder);
	}
}

static int is_motor_on(uint8_t encoder)
{
	if (encoder == ENCODER_RIGHT) {
		GPIO_PinState in1 = HAL_GPIO_ReadPin(RIGHT_MOTOR_IN1_GPIO_Port, RIGHT_MOTOR_IN1_Pin);
		GPIO_PinState in2 = HAL_GPIO_ReadPin(RIGHT_MOTOR_IN2_GPIO_Port, RIGHT_MOTOR_IN2_Pin);
		return (in1 != GPIO_PIN_RESET || in2 != GPIO_PIN_RESET) ? 1 : 0;
	}
	if (encoder < NUM_ENCODERS) {
		return motor_active[encoder] ? 1 : 0;
	}
	return 0;
}

void emulate_wheel_tick(void)
{
	for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
		if (!is_motor_on(i)) {
			continue;
		}

		int32_t delta = 0;
		switch (wheel_direction[i]) {
			case FORWARD:
			case ROTATING_CLOCK:
				delta = 1;
				break;
			case BACKWARD:
			case ROTATING_COUNTER:
				delta = -1;
				break;
			default:
				continue;
		}

		encoder_position[i] += delta;
		pulse_count[i] = (uint32_t)abs(encoder_position[i]);
		check_encoder_target(i);
	}
}

void measure_encoders(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        uint8_t encoder = 0xFF;
        GPIO_TypeDef* b_port = NULL;
        uint16_t b_pin = 0;

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            encoder = ENCODER_RIGHT;
            b_port = GPIOA;
            b_pin  = GPIO_PIN_4; /* A4 (PA4) encoder channel B */
        }
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            encoder = ENCODER_LEFT;
            b_port = GPIOA;
            b_pin  = GPIO_PIN_5; // Encoder left pin B
        }

        if (encoder < NUM_ENCODERS) {
            uint8_t b_state = HAL_GPIO_ReadPin(b_port, b_pin);

            if (b_state == 0)
                encoder_position[encoder]++;
            else
                encoder_position[encoder]--;

            pulse_count[encoder] = (uint32_t)abs(encoder_position[encoder]);
            check_encoder_target(encoder);
        }
    }
}

//Function uset to set the target to count external events (encoders)
void set_target_count(uint8_t channel, uint32_t target) {
    if (channel < NUM_CHANNELS) {
        target_count[channel] = target;
        pulse_count[channel] = 0;
        encoder_position[channel] = 0;
    }
}

int32_t get_encoder_position(uint8_t encoder) {
    if (encoder < NUM_ENCODERS)
        return encoder_position[encoder];
    return 0;
}

void get_wheel_status(uint8_t status[NUM_ENCODERS]) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        status[i] = wheel_status[i];
    }
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
