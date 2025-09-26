static bool bumpers[8];        	// FRONT, REAR, RIGHT, LEFT
    								// FRONT_RIGHT, FRONT_LEFT
									// REAR_RIGHT, REAR_LEFT, RIGHT, LEFT
    								// 1 = hit, 0 = no collision
									// FRONT, REAR, LEFT, RIGHT in cm
									// 1 = rain, pause mowing
									// 1 = ON, 0 = OFF

static uint8_t *irda_distance[4];
static uint8_t rain_detected;
static uint8_t blade_motor_status;
static volatile uint8_t *blade_speed_rpm;

void sensor_init(law_mower_status* law_mower) {
	law_mower->bumpers[FRONT] = &bumper[FRONT];
	law_mower->bumpers[REAR] = &bumper[REAR];
	law_mower->bumpers[RIGHT] = &bumper[RIGHT];
	law_mower->bumpers[LEFT] = &bumper[LEFT];
	law_mower->bumpers[FRONT_RIGHT] = &bumper[FRONT_RIGHT];
	law_mower->bumpers[FRONT_LEFT] = &bumper[FRONT_LEFT];
	law_mower->bumpers[REAR_RIGHT] = &bumper[REAR_RIGHT];
	law_mower->bumpers[REAR_LEFT] = &bumper[REAR_LEFT];
	return 0;
}

int read_bumpers(TIM_HandleTypeDef *htim) {
	//TODO implement the interrupt to read rise and drop edge
	if (HAL_GPIO_ReadPin(GPIOB, BUMPER_FRONT_Pin)) {

	}

	if (HAL_GPIO_ReadPin(GPIOB, BUMPER_REAR_Pin)) {

	}

	if (HAL_GPIO_ReadPin(GPIOB, BUMPER_RIGHT_Pin)) {

	}

	if (HAL_GPIO_ReadPin(GPIOB, BUMPER_LEFT_Pin)) {

	}

	if (HAL_GPIO_ReadPin(GPIOB, BUMPER_FRONT_RIGHT_Pin)) {

	}

	if (HAL_GPIO_ReadPin(BUMPER_FRONT_LEFT_GPIO_Port, BUMPER_FRONT_LEFT_Pin)) {

	}

	if (HAL_GPIO_ReadPin(BUMPER_REAR_RIGHT_GPIO_Port, BUMPER_REAR_RIGHT_Pin)) {

	}

	if (HAL_GPIO_ReadPin(GPIOC, BUMPER_REAR_LEFT_Pin)) {

	}

}

void read_irda_distance(void) {
	//TODO implement this function
}

void read_rain_sensor(void) {
	//TODO implement this function
}

void blade_motor_status(void) {
	//TODO implement this function blade motor_status and blade_speed
}
