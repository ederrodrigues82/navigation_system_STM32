#include "main.h"
#include "sensor.h"
#include "controller.h" // Required for lawn_mower_status
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static bool bumpers[8];        	// FRONT, REAR, RIGHT, LEFT
    								// FRONT_RIGHT, FRONT_LEFT
									// REAR_RIGHT, REAR_LEFT, RIGHT, LEFT
    								// 1 = hit, 0 = no collision
									// FRONT, REAR, LEFT, RIGHT in cm
									// 1 = rain, pause mowing
									// 1 = ON, 0 = OFF

//static uint8_t *irda_distance[4];
//static uint8_t rain_detected;
//static uint8_t blade_motor_status_func_placeholder; // Placeholder for future function
/*
	//TODO implement this function blade motor_status and blade_speed
}*/
//static volatile uint8_t *blade_speed_rpm;
/* USER CODE END PV */

void sensor_init(lawn_mower_status* law_mower) {
	law_mower->bumpers[FRONT] = &bumpers[FRONT];
	law_mower->bumpers[REAR] = &bumpers[REAR];
	law_mower->bumpers[RIGHT] = &bumpers[RIGHT];
	law_mower->bumpers[LEFT] = &bumpers[LEFT];
	law_mower->bumpers[FRONT_RIGHT] = &bumpers[FRONT_RIGHT];
	law_mower->bumpers[FRONT_LEFT] = &bumpers[FRONT_LEFT];
	law_mower->bumpers[REAR_RIGHT] = &bumpers[REAR_RIGHT];
	law_mower->bumpers[REAR_LEFT] = &bumpers[REAR_LEFT];
}

void read_bumpers(TIM_HandleTypeDef *htim) {
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
