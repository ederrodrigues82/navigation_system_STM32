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

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void start_tim2(uint8_t channel);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MOV_SIM_CHANNEL_B_Pin GPIO_PIN_14
#define MOV_SIM_CHANNEL_B_GPIO_Port GPIOC
#define MOV_SIM_CHANNEL_A_Pin GPIO_PIN_15
#define MOV_SIM_CHANNEL_A_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
#define NUM_CHANNELS 4
#define X 0
#define Y 1
#define Z 2
#define ROLL 0
#define PITCH 1
#define YAW 2
#define FRONT 0
#define REAR 1
#define LEFT 2
#define RIGHT 3
#define FRONT_RIGHT 4
#define FRONT_LEFT 5
#define REAR_RIGHT 6
#define REAR_LEFT 7

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
