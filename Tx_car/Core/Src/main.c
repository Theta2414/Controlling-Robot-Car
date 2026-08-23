/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <string.h>

#include "kalman.h"
#include "mpu6050.h"
#include "control.h"
#include "hc05.h"
#include "protocol.h"
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

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
MPU6050_t mpu;

Kalman_t kalman_pitch;
Kalman_t kalman_roll;

Control_t control;

HC05_t hc05;

char tx_buffer[PROTOCOL_BUFFER_SIZE];

volatile float pitch_filtered = 0.0f;
volatile float roll_filtered = 0.0f;

volatile int16_t control_forward = 0;
volatile int16_t control_turn = 0;

uint32_t last_update_tick = 0;

volatile uint8_t at_rx[64] = {0};
volatile uint16_t at_rx_len = 0;
volatile HAL_StatusTypeDef at_tx_status;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void HC05_AT_Command(const char *cmd)
{
    at_rx_len = 0;

    memset((void *)at_rx, 0, sizeof(at_rx));

    at_tx_status =
        HAL_UART_Transmit(&huart1,
                          (uint8_t *)cmd,
                          strlen(cmd),
                          100);

    while (at_rx_len < sizeof(at_rx) - 1)
    {
        uint8_t byte;

        if (HAL_UART_Receive(&huart1,
                             &byte,
                             1,
                             100) == HAL_OK)
        {
            at_rx[at_rx_len++] = byte;
        }
        else
        {
            break;
        }
    }
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  HC05_Init(&hc05, &huart1);
  //HC05_AT_Command("AT+BIND=98D3,21,F875D3\r\n");
  /* Checkpoint 1 */
  {
      const uint8_t msg[] = "BOOT\r\n";
      HC05_Send(&hc05, msg, sizeof(msg) - 1);
  }

  HAL_Delay(500);


  if (MPU6050_Init(&mpu, &hi2c1) != MPU6050_OK)
  {
      const uint8_t msg[] = "MPU INIT ERROR\r\n";
      HC05_Send(&hc05, msg, sizeof(msg) - 1);

      Error_Handler();
  }

  /* Checkpoint 2 */
  {
      const uint8_t msg[] = "MPU INIT OK\r\n";
      HC05_Send(&hc05, msg, sizeof(msg) - 1);
  }


  HAL_Delay(500);


  if (MPU6050_Calibrate(&mpu) != MPU6050_OK)
  {
      const uint8_t msg[] = "CALIB ERROR\r\n";
      HC05_Send(&hc05, msg, sizeof(msg) - 1);

      Error_Handler();
  }

  /* Checkpoint 3 */
  {
      const uint8_t msg[] = "CALIB OK\r\n";
      HC05_Send(&hc05, msg, sizeof(msg) - 1);
  }


  Kalman_Init(&kalman_pitch);
  Kalman_Init(&kalman_roll);

  last_update_tick = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (MPU6050_Read(&mpu) == MPU6050_OK)
    {
        uint32_t now = HAL_GetTick();

        float dt =
            (float)(now - last_update_tick) / 1000.0f;

        last_update_tick = now;

        /*
        * Protect Kalman from an invalid dt,
        * for example after debugging or a long pause.
        */
        if (dt <= 0.0f || dt > 0.1f)
        {
            dt = 0.01f;
        }


        pitch_filtered =
            Kalman_Update(&kalman_pitch,
                          mpu.pitch_acc,
                          mpu.gyro_y_dps,
                          dt);

        roll_filtered =
            Kalman_Update(&kalman_roll,
                          mpu.roll_acc,
                          mpu.gyro_x_dps,
                          dt);


        Control_Update(&control,
                      pitch_filtered,
                      roll_filtered);

        control_forward = control.forward;
        control_turn = control.turn;

        uint16_t tx_length = Protocol_Encode(control_forward, control_turn, tx_buffer, sizeof(tx_buffer));
        if (tx_length > 0)
        {
            HC05_Send(&hc05, (const uint8_t *)tx_buffer, tx_length);
        }

    }

    HAL_Delay(10);
    //HAL_Delay(1000);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
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
  huart1.Init.BaudRate = 9600;
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
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
