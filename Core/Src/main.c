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
  *
  *
  * Use code with caution.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usart_config.h"
#include "gpio.h"
#include "bmp2_config.h"
#include "heater_config.h"
#include "fan_config.h"
#include "pid_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Temperature control limits
#define MIN_TEMP 22.0f
#define MAX_TEMP 32.5f

/* USER CODE BEGIN PV */

uint8_t rx_buffer[20];
float target_temperature = 32.0f;
float current_temperature = 0.0f;
uint8_t temp_msg_buffer[100];
uint32_t last_temp_print_time = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
uint16_t calculate_crc(const uint8_t *data, size_t length);
void process_user_input(void);

/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    return (HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY) == HAL_OK) ? len : -1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        process_user_input();
        HAL_UART_Receive_IT(&huart3, rx_buffer, sizeof(rx_buffer)); // Restart reception
    }
}

uint16_t calculate_crc(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void process_user_input(void)
{
    char input[20];
    memcpy(input, rx_buffer, sizeof(rx_buffer));
    float new_temp = atof(input);

    if (new_temp >= MIN_TEMP && new_temp <= MAX_TEMP) {
        target_temperature = new_temp;
        printf("Target temperature set to: %.2f\r\n", target_temperature);
    } else {
        printf("Invalid temperature! Please enter a value between %.1f and %.1f\r\n", MIN_TEMP, MAX_TEMP);
    }
}

/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_SPI4_Init();
    MX_TIM2_Init();
    MX_TIM7_Init();
    MX_TIM3_Init();

    BMP2_Init(&bmp2dev);
    HEATER_PWM_Init(&hheater);
    FAN_PWM_Init(&hfan);
    HAL_UART_Receive_IT(&huart3, rx_buffer, sizeof(rx_buffer));
    HAL_TIM_Base_Start(&htim7);
    PID_Init();
    pid_controller.target_temperature = target_temperature;

    while (1) {
        if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
            __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

            double temp;
            BMP2_ReadData(&bmp2dev, NULL, &temp);
            current_temperature = (float)temp;

            // Update target temperature for the single controller
            pid_controller.target_temperature = target_temperature;

            // Feedforward
            float feedforward = 0.5f * (target_temperature - MIN_TEMP);

            // PID Control
            PID_Update(current_temperature);
            pid_controller.output = PID_Calculate();

            if (current_temperature < target_temperature) {
                // Heating
                pid_controller.output_saturated = fminf(fmaxf(pid_controller.output + feedforward, 0.0f), 100.0f);
                HEATER_PWM_WriteDuty(&hheater, 5 * pid_controller.output_saturated);
                FAN_PWM_WriteDuty(&hfan, 0.0f); // Fan off during heating
            } else {
                // Cooling
                pid_controller.output_saturated = fminf(fmaxf(-pid_controller.output, 0.0f), 100.0f);
                FAN_PWM_WriteDuty(&hfan, 3 * pid_controller.output_saturated);
                HEATER_PWM_WriteDuty(&hheater, 0.0f); // Heater off during cooling
            }

            // Send Temperature Data with CRC
            if (HAL_GetTick() - last_temp_print_time >= 1000) {
                int msg_len = snprintf((char *)temp_msg_buffer, sizeof(temp_msg_buffer),
                                       "{\"id\":1, \"target_temp\":%.2f, \"temp\":%.2f, \"pid_output\":%.2f}",
                                       target_temperature, current_temperature, pid_controller.output);

                uint16_t crc = calculate_crc(temp_msg_buffer, msg_len);
                snprintf((char *)temp_msg_buffer + msg_len, sizeof(temp_msg_buffer) - msg_len, ", \"crc\":%04X}\r\n", crc);

                HAL_UART_Transmit(&huart3, temp_msg_buffer, strlen((char *)temp_msg_buffer), HAL_MAX_DELAY);
                last_temp_print_time = HAL_GetTick();
            }
        }

        HAL_Delay(100);
        /* USER CODE END WHILE */
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
void process_uart_message(const char *msg)
{
    char *target_temp_str = strstr(msg, "\"target_temp\":"); // Correct string with escaped quotes
    if (target_temp_str != NULL) {
        target_temp_str += 14; // Move to the start of the value
        float new_temp = atof(target_temp_str);

        if (new_temp >= MIN_TEMP && new_temp <= MAX_TEMP) {
            target_temperature = new_temp;
            printf("Target temperature set to: %.2f\r\n", target_temperature);
        } else {
            printf("Invalid temperature! Please enter a value between %.1f and %.1f\r\n", MIN_TEMP, MAX_TEMP);
        }
    }
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
