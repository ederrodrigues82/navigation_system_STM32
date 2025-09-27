/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "controller.h"
#include "menu.h"
#include "mov_simulator.h"
#include "actuators.h"
#include "communication.h"
#include "sensor.h"
#include "positioning.h"
#include "communication_test.h" // Include for SPI communication test function
#include <stdarg.h> // Required for variadic functions
#include <stdio.h>  // Required for vsnprintf

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
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
// SPI_HandleTypeDef hspi1; // Already declared above
lawn_mower_status m_status;
flat_lawn_mower_status flat_m_status;

uint8_t rx_data = 0; // Definition of rx_data

// Remove these placeholder variables as lawn_mower_status members are no longer pointers.
// int32_t current_left_motor_speed = 0;
// int32_t current_right_motor_speed = 0;
// uint32_t current_left_encoder_count = 0;
// uint32_t current_right_encoder_count = 0;
// float current_speed_mps = 0.0f;
// float current_heading_deg = 0.0f;
// float current_pos[3] = {0.0f, 0.0f, 0.0f};
// float current_accel[3] = {0.0f, 0.0f, 0.0f};
// float current_gyro[3] = {0.0f, 0.0f, 0.0f};
// float current_euler_angles[3] = {0.0f, 0.0f, 0.0f};
// bool current_bumpers[8] = {false};
// uint8_t current_irda_distance[4] = {0};
// uint8_t current_blade_speed_rpm = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C2_SMBUS_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
//static void BNO080_activate(void);
//static void UpdateVelocityPosition(float dt);
//static void BNO080_ParseInputReport(uint8_t *data);
float FilteredAccel(float *buffer, float newValue);
float HighPassFilter(float current_input, float previous_output, float alpha);
void QuaternionToEuler(float w, float x, float y, float z, float *roll, float *pitch, float *yaw);

// Forward declarations for callbacks
void irda_motor_callback(void);
void blade_motor_callback(void);
void encoder_left_callback(void);
void encoder_right_callback(void);

// Array of function pointers to callbacks
void (*channel_callbacks[NUM_CHANNELS])(void) = {
    encoder_right_callback,
    encoder_left_callback,
    blade_motor_callback,
    irda_motor_callback
};
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdarg.h> // Required for variadic functions
#include <stdio.h>  // Required for vsnprintf

// Buffer for formatted log messages
#define LOG_BUFFER_SIZE 256
static char log_buffer[LOG_BUFFER_SIZE];

// UART Handle (declared in main.c, extern in main.h)
extern UART_HandleTypeDef huart1;

/**
  * @brief  Custom logging function to send formatted messages over UART.
  * @param  level: The log level (DEBUG, INFO, WARNING, ERROR, CRITICAL).
  * @param  file: The source file where the log was made.
  * @param  line: The line number in the source file.
  * @param  format: Format string for the message.
  * @retval None
  */
void log_message(log_level_t level, const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Get current tick for timestamp
    uint32_t timestamp = HAL_GetTick();

    // Determine log level string
    const char* level_str;
    switch (level) {
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            break;
        case LOG_LEVEL_WARNING:
            level_str = "WARNING";
            break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        case LOG_LEVEL_CRITICAL:
            level_str = "CRITICAL";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    // Format the log message
    int len = snprintf(
        log_buffer, LOG_BUFFER_SIZE,
        "%lu - %s:%d - %s - ",
        timestamp, file, line, level_str
    );

    if (len < 0 || len >= LOG_BUFFER_SIZE) {
        // Handle error or truncation
        len = LOG_BUFFER_SIZE - 1;
    }

    vsnprintf(log_buffer + len, LOG_BUFFER_SIZE - len, format, args);
    va_end(args);

    // Add newline and send over UART
    size_t total_len = strlen(log_buffer);
    if (total_len < LOG_BUFFER_SIZE - 1) {
        strcat(log_buffer, "\r\n");
        total_len += 2;
    } else if (total_len == LOG_BUFFER_SIZE - 1) {
        // Buffer full, just ensure null termination for current message
        // No space for newline, or it would overwrite existing chars.
        log_buffer[LOG_BUFFER_SIZE - 1] = '\0';
    }

    HAL_UART_Transmit(&huart1, (uint8_t*)log_buffer, total_len, HAL_MAX_DELAY);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_I2C2_SMBUS_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init(); // This call is crucial for UART to work
  /* USER CODE BEGIN 2 */
  BNO080_activate();
  HAL_TIM_Base_Start_IT(&htim3);
//  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  LOG_INFO("Initialization was successful!");
  menu(rx_data);

  // Initialize m_status members with default values
  m_status.left_motor_speed = 0;
  m_status.right_motor_speed = 0;
  m_status.left_encoder_count = 0;
  m_status.right_encoder_count = 0;
  m_status.speed_mps = 0.0f;
  m_status.heading_deg = 0.0f;
  m_status.blade_speed_rpm = 0;
  
  for(int i = 0; i < 3; i++) {
      m_status.pos[i] = 0.0f;
      m_status.accel[i] = 0.0f;
      m_status.gyro[i] = 0.0f;
      m_status.euler_angles[i] = 0.0f;
  }

  for(int i = 0; i < 8; i++) {
      m_status.bumpers[i] = false;
  }
  for(int i = 0; i < 4; i++) {
      m_status.irda_distance[i] = 0;
  }
  strncpy(m_status.direction, "STOP", sizeof(m_status.direction) - 1);
  m_status.direction[sizeof(m_status.direction) - 1] = '\0';
  m_status.rain_detected = 0;
  m_status.blade_motor_status = 0;
  m_status.battery_voltage = 0.0f;
  m_status.battery_current = 0.0f;
  m_status.battery_percentage = 0;
  m_status.charging_status = 0;
  m_status.uptime_ms = 0;
  m_status.error_code = 0;
  m_status.is_manual_mode = 0;
  m_status.is_emergency_stop = 0;
  m_status.task_state = 0; // Assuming MowerState enum starts from 0
  m_status.edge_sensor = 0; // Assuming Edge_sensor enum starts from 0

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      // For testing purposes: Mock data and send over SPI
	  send_ping_message(); // <--- Add this line here
	  //run_spi_test(&m_status);
      // The following lines are now handled within run_spi_test for testing. 
      // In a real application, you would manage serialization and transmission here based on your logic.
      /*
      serialize_lawn_mower_status(&m_status, &flat_m_status);
      uint8_t rx_buffer[sizeof(flat_lawn_mower_status)];
      HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&flat_m_status, rx_buffer, sizeof(flat_m_status), HAL_MAX_DELAY);
      */
      //HAL_Delay(10); // Transmit every 10ms, adjust as needed for speed
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_SMBUS_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  LOG_INFO("Timer_3 initialized successfully!");
  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, BUMPER_REAR_LEFT_Pin|MOV_SIM_CHANNEL_B_Pin|MOV_SIM_CHANNEL_A_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUMPER_FRONT_LEFT_GPIO_Port, BUMPER_FRONT_LEFT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUMPER_REAR_LEFT_Pin MOV_SIM_CHANNEL_B_Pin MOV_SIM_CHANNEL_A_Pin */
  GPIO_InitStruct.Pin = BUMPER_REAR_LEFT_Pin|MOV_SIM_CHANNEL_B_Pin|MOV_SIM_CHANNEL_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BUMPER_FRONT_Pin BUMPER_REAR_Pin BUMPER_RIGHT_Pin BUMPER_LEFT_Pin
                           BUMPER_FRONT_RIGHT_Pin */
  GPIO_InitStruct.Pin = BUMPER_FRONT_Pin|BUMPER_REAR_Pin|BUMPER_RIGHT_Pin|BUMPER_LEFT_Pin
                          |BUMPER_FRONT_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BUMPER_FRONT_LEFT_Pin */
  GPIO_InitStruct.Pin = BUMPER_FRONT_LEFT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUMPER_FRONT_LEFT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUMPER_REAR_RIGHT_Pin */
  GPIO_InitStruct.Pin = BUMPER_REAR_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUMPER_REAR_RIGHT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//DMA Callback
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        BNO080_ParseInputReport(bno080_rx_buffer);
        BNO080_Receive();  // Re-arm DMA for next packet
    }
}

// Handler for external events (encoders)
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
//	printf("TIM2_IRQHandler\r\n");
	if (htim->Instance == TIM2) {
		if ((htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) ||
				(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)) {
			measure_encoders(htim);
		} else {
			read_bumpers(htim);
		}
	}
}

void start_tim2(uint8_t channel) {
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1 + channel * 4); // Start only that channel
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if (htim->Instance == TIM3)
//    {
//        tick();  // call simulator tick function
//        //LOG_INFO("Timer_3 Tick!");
//    }
//}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	//LOG_INFO("Receive data from Uart");
//    {
//	if (huart->Instance == USART1)
//        // Exemplo: eco do caractere recebido
//        //HAL_UART_Transmit(&huart1, &rx_data, 1, HAL_MAX_DELAY);
//        //LOG_INFO("Receive data from Uart");
//        menu(rx_data);
//
//        // Reinicia a recepção do próximo byte
//        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
//    }
//}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  LOG_CRITICAL("Error_Handler called!");
//  /* User can add his own implementation to report the HAL error return state */
//  __disable_irq();
//  while (1)
//  {
//  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

// Ping message function for UART testing
void send_ping_message(void) {
    static uint32_t last_ping_time = 0;
    if (HAL_GetTick() - last_ping_time >= 2000) { // Every 2 seconds
        LOG_INFO("STM32 Ping! Uptime: %lu ms", HAL_GetTick());
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the onboard LED (PC13)
        last_ping_time = HAL_GetTick();
    }
}
