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
#include "fatfs.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmp2_config.h"
#include "heater_config.h"
#include "fan_config.h"
#include "pid_control.h"
#include "fatfs.h"
#include "LCD_HD44780.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIN_TEMP 22.0f
#define MAX_TEMP 36.5f
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_buffer[20];
float target_temperature = 30.0f;
float current_temperature = 0.0f;
uint8_t temp_msg_buffer[100];
uint32_t last_temp_print_time = 0;
SPI_HandleTypeDef hspi5;
FATFS fs; 
FATFS *pfs; 
FIL fil; 
FRESULT fres;
DWORD fre_clust;
char buffer[100]; // for LCD

uint32_t Numbah = 0;   // counting first column
float dur = 0.0;  //time duration

volatile _Bool USER_Btn_RisingEdgeDetected = 0; //both to help tell when to dismount SD card
GPIO_PinState LD1_State = GPIO_PIN_RESET; 

uint8_t degree_symbol[8] = { // the degree simbol on LCD 
    0b00110,
    0b01001,
    0b01001,
    0b00110,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint16_t calculate_crc(const uint8_t *data, size_t length);
void process_user_input(void);
void format_time(uint32_t ticks, char *time_buffer, size_t buffer_size);
void send_temperature_data_with_time();
void process_uart_message(const char *msg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

int _write(int file, char *ptr, int len)
{
  int DataIdx;
  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
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

void format_time(uint32_t ticks, char *time_buffer, size_t buffer_size)
{
    uint32_t total_seconds = ticks / 1000;
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;

    snprintf(time_buffer, buffer_size, "%02lu:%02lu:%02lu", hours, minutes, seconds);
}

void send_temperature_data_with_time()
{
    if (HAL_GetTick() - last_temp_print_time >= 1000) {
        char time_buffer[10];
        format_time(HAL_GetTick(), time_buffer, sizeof(time_buffer)); // Get formatted time string

        int msg_len = snprintf((char *)temp_msg_buffer, sizeof(temp_msg_buffer),
                               "{\"id\":1, \"time\":\"%s\", \"target_temp\":%.2f, \"temp\":%.2f, \"pid_output\":%.2f",
                               time_buffer, target_temperature, current_temperature, pid_controller.output);

        uint16_t crc = calculate_crc(temp_msg_buffer, msg_len);
        snprintf((char *)temp_msg_buffer + msg_len, sizeof(temp_msg_buffer) - msg_len, ", \"crc\":%04X}\r\n", crc);

        HAL_UART_Transmit(&huart3, temp_msg_buffer, strlen((char *)temp_msg_buffer), HAL_MAX_DELAY);
        last_temp_print_time = HAL_GetTick();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == USER_Btn_Pin)
    USER_Btn_RisingEdgeDetected = 1;
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
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
    BMP2_Init(&bmp2dev);
    HEATER_PWM_Init(&hheater);
    FAN_PWM_Init(&hfan);
    HAL_UART_Receive_IT(&huart3, rx_buffer, sizeof(rx_buffer));
    HAL_TIM_Base_Start(&htim7);
    PID_Init();
    pid_controller.target_temperature = target_temperature;

    LCD_Init();
    LCD_Clear();
    LCD_CreateCustomChar(degree_symbol, 0);

    HAL_Delay(500);

    fres = f_mount(&fs, "", 0); //mounting SD
    fres = f_open(&fil, "temp_log.csv", FA_CREATE_ALWAYS | FA_WRITE); // creating file
    sprintf(buffer, "#,Time Duration[sek],Targer Temp[C],Current Temp[C]\n"); //creating heders
    f_puts(buffer, &fil); //printing

    char line1[17];
    char line2[17];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
        if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
            __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

            double temp;
            BMP2_ReadData(&bmp2dev, NULL, &temp);
            current_temperature = (float)temp;

            __io_putchar('H');
            __io_putchar('e');
            __io_putchar('l');
            __io_putchar('l');
            __io_putchar('o');
            __io_putchar('\r');
            __io_putchar('\n');

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
                pid_controller.output_saturated = fminf(fmaxf(abs(pid_controller.output), 0.0f), 100.0f);
                FAN_PWM_WriteDuty(&hfan, 3 * pid_controller.output_saturated);
                HEATER_PWM_WriteDuty(&hheater, 0.0f); // Heater off during cooling
            }

            // Send Temperature Data with CRC
            send_temperature_data_with_time();
        }

        snprintf(line1, sizeof(line1), "TarTemp: %.1f ", target_temperature);
        snprintf(line2, sizeof(line2), "CurTemp: %.1f ", current_temperature);
        LCD_SetCursor(0, 0);
        LCD_Print(line1);
        LCD_WriteData(0);
        LCD_Print("C");
        LCD_SetCursor(1, 0);
        LCD_Print(line2);
        LCD_WriteData(0);
        LCD_Print("C");

        sprintf(buffer, "%ld,%.1f,%.1f,%.1f\n", Numbah, dur, target_temperature, current_temperature);
        f_puts(buffer, &fil);

        Numbah += 1;
        dur += 0.1;

        if(USER_Btn_RisingEdgeDetected) //saving and dismounting card
        {
            HAL_Delay(10);
            USER_Btn_RisingEdgeDetected = 0;
            if(HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_SET) //button pressed
            {
                HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin); //led1 on
                fres = f_close(&fil); //file closed
                f_mount(NULL, "", 1); //card dismounted
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
