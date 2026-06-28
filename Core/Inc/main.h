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

// Log levels, similar to Python's logging module
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL
} log_level_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define NUM_ENCODERS 2
#define EMULATE_WHEEL_TICK_MS 200
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void log_message(log_level_t level, const char* file, int line, const char* format, ...);
#define LOG_DEBUG(format, ...)    log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)     log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...)  log_message(LOG_LEVEL_WARNING, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)    log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_CRITICAL(format, ...) log_message(LOG_LEVEL_CRITICAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

void start_tim2(uint8_t channel);
extern uint8_t rx_data;
void send_ping_message(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUMPER_REAR_LEFT_Pin GPIO_PIN_13
#define BUMPER_REAR_LEFT_GPIO_Port GPIOC
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

/* Right wheel motor (H-bridge IN1, IN2) */
#define RIGHT_MOTOR_IN1_Pin GPIO_PIN_10
#define RIGHT_MOTOR_IN1_GPIO_Port GPIOB
#define RIGHT_MOTOR_IN2_Pin GPIO_PIN_11
#define RIGHT_MOTOR_IN2_GPIO_Port GPIOB

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
