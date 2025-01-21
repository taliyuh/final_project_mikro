/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp2_config.h"
#include "heater_config.h"
#include "fan_config.h"

// PID parameters
#define PID_KP_HEAT 0.8f
#define PID_KI_HEAT 0.1f
#define PID_KD_HEAT 0.02f

#define PID_KP_COOL 0.4f
#define PID_KI_COOL 0.02f
#define PID_KD_COOL 0.01f

// Temperature control limits
#define MIN_TEMP 25.0f
#define MAX_TEMP 30.0f

// Fan parameters
#define FAN_MIN_DUTY 60.0f

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
uint8_t rx_buffer[20]; // Buffer for receiving user input
const int rx_msg_len = 20;
uint8_t temp_msg_buffer[50]; // Buffer for temperature messages
uint32_t last_temp_print_time = 0; // To track when the last temperature message was printed

float target_temperature = MIN_TEMP;
float current_temperature = 0.0f;
float pid_error = 0.0f;
float pid_integral = 0.0f;
float pid_derivative = 0.0f;
float pid_output = 0.0f;
float previous_error = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void process_user_input(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
  return (HAL_UART_Transmit(&huart3, (uint8_t*)ptr, len, HAL_MAX_DELAY) == HAL_OK) ? len : -1;
}

/**
 * @brief  Rx Transfer completed callback.
 * @param  huart UART handle.
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        // Check for a specific command to change the target temperature
        if (strncmp((char*)rx_buffer, "settemp:", 8) == 0)
        {
            process_user_input();
        }

        // Restart the UART receive with the maximum buffer size
        HAL_UART_Receive_IT(&huart3, rx_buffer, sizeof(rx_buffer));
    }
}

void process_user_input() {
    float new_target_temp;

    // Attempt to parse the new target temperature
    if (sscanf((char*)rx_buffer, "settemp:%f", &new_target_temp) == 1) {
        if (new_target_temp >= MIN_TEMP && new_target_temp <= MAX_TEMP) {
            target_temperature = new_target_temp;
            printf("Target temperature updated to: %.1f °C\r\n", target_temperature);
        } else {
            printf("Invalid temperature. Please enter a value between %.1f and %.1f °C.\r\n", MIN_TEMP, MAX_TEMP);
        }
    } else {
        printf("Invalid command format. Use 'settemp:XX.X'.\r\n");
    }

    // Clear the buffer after processing
    memset(rx_buffer, 0, sizeof(rx_buffer));
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
  MX_USART3_UART_Init();
  MX_SPI4_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  BMP2_Init(&bmp2dev);
  HEATER_PWM_Init(&hheater);
  FAN_PWM_Init(&hfan);
  HAL_UART_Receive_IT(&huart3, rx_buffer, sizeof(rx_buffer)); // Start receiving user input
  HAL_TIM_Base_Start(&htim7);

  // No initial user input needed here

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE))
    {
        __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

        double temp;
        BMP2_ReadData(&bmp2dev, NULL, &temp);
        current_temperature = (float)temp;

        // PID Control
        pid_error = target_temperature - current_temperature;
        pid_integral += pid_error;
        pid_derivative = pid_error - previous_error;
        previous_error = pid_error;

        // Control Logic
        if (current_temperature < target_temperature) {
            // Heating
            pid_output = (PID_KP_HEAT * pid_error) + (PID_KI_HEAT * pid_integral) + (PID_KD_HEAT * pid_derivative);
            float heater_duty = pid_output;
            if (heater_duty < 0.0f) heater_duty = 0.0f;
            if (heater_duty > 100.0f) heater_duty = 100.0f;

            HEATER_PWM_WriteDuty(&hheater, 5*heater_duty);
            FAN_PWM_WriteDuty(&hfan, 0.0f);
        } else {
            // Cooling
            pid_output = (PID_KP_COOL * pid_error) + (PID_KI_COOL * pid_integral) + (PID_KD_COOL * pid_derivative);
            HEATER_PWM_WriteDuty(&hheater, 0.0f);

            float fan_duty = -pid_output;
            if (fan_duty < 0.0f) fan_duty = 0.0f;
            if (fan_duty > 100.0f) fan_duty = 100.0f;

            FAN_PWM_WriteDuty(&hfan, fan_duty);
        }

        // Print temperature every 5 seconds
        if (HAL_GetTick() - last_temp_print_time >= 5000) {
            sprintf((char*)temp_msg_buffer, "{\"id\":1, \"target_temp\":%5.2f, \"temp\":%5.2f, \"pid_output\":%5.2f}\r\n", target_temperature, current_temperature, pid_output);
            HAL_UART_Transmit(&huart3, temp_msg_buffer, strlen((char*)temp_msg_buffer), HAL_MAX_DELAY);
            last_temp_print_time = HAL_GetTick();
        }
    }

    HAL_Delay(100); // Adjust control loop frequency if needed
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
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
