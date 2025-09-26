/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
// Keep HAL_SPI_Init include here as it's a HAL driver
#include "stm32f1xx_hal_spi.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// All project-specific includes and other standard C headers will be moved to the respective .c files or other .h files.
#include <stdbool.h> // Required for bool type
#include <string.h>  // Required for strncpy
#include <math.h>    // Required for math functions like atan2f, fabsf, copysignf, asinf, M_PI
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
    FRONT,
	REAR,
	RIGHT,
	LEFT,
	FRONT_RIGHT,
	FRONT_LEFT,
	REAR_RIGHT,
	REAR_LEFT
} Direction;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define NUM_ENCODERS 2
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void start_tim2(uint8_t channel);
extern uint8_t rx_data;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUMPER_REAR_LEFT_Pin GPIO_PIN_13
#define BUMPER_REAR_LEFT_GPIO_Port GPIOC
#define MOV_SIM_CHANNEL_B_Pin GPIO_PIN_14
#define MOV_SIM_CHANNEL_B_GPIO_Port GPIOC
#define MOV_SIM_CHANNEL_A_Pin GPIO_PIN_15
#define MOV_SIM_CHANNEL_A_GPIO_Port GPIOC
#define BUMPER_FRONT_Pin GPIO_PIN_0
#define BUMPER_FRONT_GPIO_Port GPIOB
#define BUMPER_REAR_Pin GPIO_PIN_1
#define BUMPER_REAR_GPIO_Port GPIOB
#define BUMPER_RIGHT_Pin GPIO_PIN_2
#define BUMPER_RIGHT_GPIO_Port GPIOB
#define BUMPER_FRONT_LEFT_Pin GPIO_PIN_13
#define BUMPER_FRONT_LEFT_GPIO_Port GPIOB
#define BUMPER_REAR_RIGHT_Pin GPIO_PIN_14
#define BUMPER_REAR_RIGHT_GPIO_Port GPIOB
#define BUMPER_LEFT_Pin GPIO_PIN_8
#define BUMPER_LEFT_GPIO_Port GPIOB
#define BUMPER_FRONT_RIGHT_Pin GPIO_PIN_9
#define BUMPER_FRONT_RIGHT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define NUM_CHANNELS 4
#define X 0
#define Y 1
#define Z 2
#define ROLL 0
#define PITCH 1
#define YAW 2


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
