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
#include <math.h>
#include <stdio.h>
#include <menu.h>
#include <mov_simulator.h>
#include <actuators.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BNO080_BUFFER_SIZE 20 //buffer size of movement sensor
#define BNO080_ADDR (0x4A << 1)  // = 0x94 ADDR pulled down
#define FILTER_SIZE 10  // Filter used on accelerometer, you can adjust this value
#define DT 100 // Period o accelerometer measure in miliseconds

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t bno080_rx_buffer[BNO080_BUFFER_SIZE];
float accelX_buffer[FILTER_SIZE] = {0};
float accelY_buffer[FILTER_SIZE] = {0};
float accelZ_buffer[FILTER_SIZE] = {0};
uint8_t accel_index = 0;
uint8_t rx_data;

typedef struct {
    float accelX, accelY, accelZ;
    float velX, velY, velZ;
    float posX, posY, posZ;
    float roll, pitch, yaw;
} BNO080_Data_t;

BNO080_Data_t bno080_data;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
static void BNO080_Receive(void);
static void BNO080_activate(void);
static void UpdateVelocityPosition(float dt);
static void BNO080_ParseInputReport(uint8_t *data);
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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  BNO080_activate();
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  printf("Initialization was successful!\r\n");
  menu(rx_data);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  printf("Timer_3 initialized with successful!\r\n");
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|MOV_SIM_CHANNEL_B_Pin|MOV_SIM_CHANNEL_A_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 MOV_SIM_CHANNEL_B_Pin MOV_SIM_CHANNEL_A_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_13|MOV_SIM_CHANNEL_B_Pin|MOV_SIM_CHANNEL_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
	measure_encoders(htim);
}

void start_tim2(uint8_t channel) {
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1 + channel * 4); // Start only that channel
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        tick();  // call simulator tick function
        //printf("Timer_3 Tick!\r\n");
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//printf("Receive data from Uart");
	if (huart->Instance == USART1)
    {
        // Exemplo: eco do caractere recebido
        //HAL_UART_Transmit(&huart1, &rx_data, 1, HAL_MAX_DELAY);
        //printf("Receive data from Uart");
        menu(rx_data);

        // Reinicia a recepção do próximo byte
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
}

// Configure and transmit message to BNO080
static void BNO080_SetFeatureCommand(uint8_t reportID, uint16_t interval)
{
    uint8_t packet[17] = {0};

    packet[0] = 0xFD; // Start byte
    packet[1] = 0x0F; // Packet length (LSB)
    packet[2] = 0x00; // Packet length (MSB)
    packet[3] = 0xFD; // Channel (always 0xFD)
    packet[4] = 0x05; // Command: Set Feature Command
    packet[5] = reportID; // e.g., 0x01 for Accelerometer
    packet[6] = 0x00; // Feature flags
    packet[7] = interval & 0xFF; // Report interval LSB
    packet[8] = (interval >> 8) & 0xFF; // MSB
    // The rest are defaults or zeros...

    HAL_I2C_Master_Transmit(&hi2c1, BNO080_ADDR, packet, 17, HAL_MAX_DELAY);
}

//Activate accelerometer and rotation vector
static void BNO080_activate(void) {
	//BNO080_SetFeatureCommand(0x01, 100); // Accelerometer, 100 ms
	BNO080_SetFeatureCommand(0x04, DT); // Linear Acceleration
	BNO080_SetFeatureCommand(0x05, DT); // Rotation Vector, 100 ms
}

static void BNO080_Receive(void)
{
    HAL_I2C_Master_Receive_DMA(&hi2c1, BNO080_ADDR, bno080_rx_buffer, sizeof(bno080_rx_buffer));
}

//Parse BNO080 data, accelerometer and rotation vector
static void BNO080_ParseInputReport(uint8_t *data) {
    static float prevAccelX = 0, prevAccelY = 0, prevAccelZ = 0;

    uint8_t reportID = data[4];
    if (reportID == 0x04) {
        int16_t x = (data[5] | (data[6] << 8));
        int16_t y = (data[7] | (data[8] << 8));
        int16_t z = (data[9] | (data[10] << 8));

        float rawX = x * 0.0001f;
        float rawY = y * 0.0001f;
        float rawZ = z * 0.0001f;

        float filteredX = FilteredAccel(accelX_buffer, rawX);
        float filteredY = FilteredAccel(accelY_buffer, rawY);
        float filteredZ = FilteredAccel(accelZ_buffer, rawZ);

        prevAccelX = bno080_data.accelX;
        prevAccelY = bno080_data.accelY;
        prevAccelZ = bno080_data.accelZ;

        bno080_data.accelX = HighPassFilter(filteredX, prevAccelX, 0.9f);
		bno080_data.accelY = HighPassFilter(filteredY, prevAccelY, 0.9f);
		bno080_data.accelZ = HighPassFilter(filteredZ, prevAccelZ, 0.9f);
		UpdateVelocityPosition(DT);

    } else if (reportID == 0x05) {
        int16_t i = (data[5] | (data[6] << 8));
        int16_t j = (data[7] | (data[8] << 8));
        int16_t k = (data[9] | (data[10] << 8));
        int16_t real = (data[11] | (data[12] << 8));
        float quatI = i * 0.0001f;
        float quatJ = j * 0.0001f;
        float quatK = k * 0.0001f;
        float quatReal = real * 0.0001f;
        QuaternionToEuler(
                quatReal,
                quatI,
                quatJ,
                quatK,
                &bno080_data.roll,
                &bno080_data.pitch,
                &bno080_data.yaw
            );
    }
}

//High pass filter to improve the sensor signal
float HighPassFilter(float current_input, float previous_output, float alpha) {
    return alpha * (previous_output + current_input - previous_output);
}

//Removes high-frequency noise well
float FilteredAccel(float *buffer, float newValue)
{

	buffer[accel_index] = newValue;

    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        sum += buffer[i];
    }

    // Wrap index for next write
    accel_index = (accel_index + 1) % FILTER_SIZE;

    return sum / FILTER_SIZE;
}

//Quartenion to Euler convertion
/**
 * Converts a quaternion (w, x, y, z) to Euler angles (roll, pitch, yaw).
 * Angles are in radians.**/
void QuaternionToEuler(float w, float x, float y, float z, float *roll, float *pitch, float *yaw)
{
    // Roll (x-axis rotation)
    *roll = atan2f(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));

    // Pitch (y-axis rotation)
    float sinp = 2.0f * (w * y - z * x);
    if (fabsf(sinp) >= 1.0f)
        *pitch = copysignf(M_PI / 2.0f, sinp);  // use 90 degrees if out of range
    else
        *pitch = asinf(sinp);

    // Yaw (z-axis rotation)
    *yaw = atan2f(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
}

static void UpdateVelocityPosition(float dt) {
    dt = DT / 1000; // convert dt to miliseconds
	bno080_data.velX += bno080_data.accelX * dt;
    bno080_data.velY += bno080_data.accelY * dt;
    bno080_data.velZ += bno080_data.accelZ * dt;

    bno080_data.posX += bno080_data.velX * dt;
    bno080_data.posY += bno080_data.velY * dt;
    bno080_data.posZ += bno080_data.velZ * dt;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
