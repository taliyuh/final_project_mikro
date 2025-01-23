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
#include <time.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp2_config.h"
#include "heater_config.h"
#include "fan_config.h"


// PID parameters for heating (from MATLAB)
#define PID_KP_HEAT 7.0f
#define PID_KI_HEAT 0.25f
#define PID_KD_HEAT 250.0f

// PID parameters for cooling (you might want to start with the same values
// and then tune them separately if needed)
#define PID_KP_COOL 10.0f
#define PID_KI_COOL 0.02f
#define PID_KD_COOL 450.0f



// Temperature control limits
#define MIN_TEMP 22.0f
#define MAX_TEMP 32.5f
#define INTEGRAL_MAX 500.0f // Example maximum value for the integral term
#define INTEGRAL_MIN -500.0f // Example minimum value for the integral term
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

float target_temperature = 29.0;//MIN_TEMP;
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
        // Process the received character immediately
        process_user_input();

        // Restart UART reception for a single character
        HAL_UART_Receive_IT(&huart3, rx_buffer, 1);
    }
}
void process_user_input() {
    static uint8_t input_buffer[20];
    static uint8_t input_index = 0;

    // Add the received character to the input buffer
    if (input_index < sizeof(input_buffer) - 1) {
        input_buffer[input_index++] = rx_buffer[0];
    }

    // Check if the received character is a newline, indicating the end of a command
    if (rx_buffer[0] == '\n' || rx_buffer[0] == '\r') {
        input_buffer[input_index] = '\0'; // Null-terminate the string

        float new_target_temp;
        if (sscanf((char*)input_buffer, "settemp:%f", &new_target_temp) == 1) {
            if (new_target_temp >= MIN_TEMP && new_target_temp <= MAX_TEMP) {
                target_temperature = new_target_temp;
                printf("Target temperature updated to: %.2f °C\r\n", target_temperature);
            } else {
                printf("Invalid temperature. Please enter a value between %.1f and %.1f °C.\r\n", MIN_TEMP, MAX_TEMP);
            }
        } else {
            printf("Invalid command format. Use 'settemp:XX.X'.\r\n");
        }

        // Reset the input buffer and index for the next command
        input_index = 0;
        memset(input_buffer, 0, sizeof(input_buffer));
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
  srand((unsigned int)time(NULL)); // Seed the random number generator
  uint32_t last_temp_update_time = HAL_GetTick(); // Track last random temperature update

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

        if (HAL_GetTick() - last_temp_update_time >= 60000) // Check if 10 seconds have passed
        {
            last_temp_update_time = HAL_GetTick();

            // Generate a random temperature within the range [MIN_TEMP, MAX_TEMP]
            float random_temp = MIN_TEMP + ((float)rand() / RAND_MAX) * (MAX_TEMP - MIN_TEMP);
            target_temperature = random_temp;
        }

        // PID Control
        // PID Control Logic Improvements
        // Anti-Windup Parameters
        float pid_output_saturated = 0.0f; // Store the saturated PID output
        const float back_calculation_gain = 0.1f; // Gain for anti-windup
        float feedforward = 0.5f * (target_temperature - MIN_TEMP); // Scale based on system characteristics
        pid_output += feedforward;

        // Derivative Filter Parameters
        const float derivative_filter_constant = 0.9f; // Adjust as needed
        float previous_derivative = 0.0f;

        if (current_temperature < target_temperature) {
            // Heating
            pid_error = target_temperature - current_temperature;

            // Integral term with anti-windup
            pid_integral += pid_error;
            if (pid_output_saturated != pid_output) { // Back-calculation
                pid_integral -= back_calculation_gain * (pid_output_saturated - pid_output);
            }
            pid_integral = fminf(fmaxf(pid_integral, INTEGRAL_MIN), INTEGRAL_MAX); // Clamp

            // Derivative term with filtering
            pid_derivative = (pid_error - previous_error);
            pid_derivative = derivative_filter_constant * previous_derivative + (1.0f - derivative_filter_constant) * pid_derivative;

            // PID output
            pid_output = (PID_KP_HEAT * pid_error) + (PID_KI_HEAT * pid_integral) + (PID_KD_HEAT * pid_derivative);
            pid_output_saturated = fminf(fmaxf(pid_output, 0.0f), 100.0f);

            HEATER_PWM_WriteDuty(&hheater, 5 * pid_output_saturated);
            FAN_PWM_WriteDuty(&hfan, 0.0f);

        } else {
            // Cooling
            pid_error = target_temperature - current_temperature;

            // Integral term with anti-windup
            pid_integral += pid_error;
            if (pid_output_saturated != pid_output) { // Back-calculation
                pid_integral -= back_calculation_gain * (pid_output_saturated - pid_output);
            }
            pid_integral = fminf(fmaxf(pid_integral, INTEGRAL_MIN), INTEGRAL_MAX); // Clamp

            // Derivative term with filtering
            pid_derivative = (pid_error - previous_error);
            pid_derivative = derivative_filter_constant * previous_derivative + (1.0f - derivative_filter_constant) * pid_derivative;

            // PID output
            pid_output = (PID_KP_COOL * pid_error) + (PID_KI_COOL * pid_integral) + (PID_KD_COOL * pid_derivative);
            pid_output_saturated = fminf(fmaxf(-pid_output, 0.0f), 100.0f);

            FAN_PWM_WriteDuty(&hfan, 3 * pid_output_saturated);
            HEATER_PWM_WriteDuty(&hheater, 0.0f);
        }

        // Update previous values
        previous_error = pid_error;
        previous_derivative = pid_derivative;

        // Print temperature every 5 seconds
        if (HAL_GetTick() - last_temp_print_time >= 1000) {
            sprintf((char*)temp_msg_buffer, "{\"id\":1, \"target_temp\":%5.2f, \"temp\":%5.2f, \"pid_output\":%5.2f}\r\n", target_temperature, current_temperature, pid_output);
            HAL_UART_Transmit(&huart3, temp_msg_buffer, strlen((char*)temp_msg_buffer), HAL_MAX_DELAY);
            last_temp_print_time = HAL_GetTick();
        }


    HAL_Delay(100); // Adjust control loop frequency if needed
    /* USER CODE END WHILE */
    }
  }
    /* USER CODE BEGIN 3 */

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
