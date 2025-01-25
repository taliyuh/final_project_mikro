/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : usart_config.h
  * @author   : taliyuh    bartlomiej.hryniewski@student.put.poznan.pl
  * @version  : 1.0.0
  * @date     : Jan 21, 2025
  * @brief    : Header file for USART3 communication functions.
  ******************************************************************************
/**
/* USER CODE END Header */

#ifndef INC_USART_CONFIG_H_
#define INC_USART_CONFIG_H_

/* Public includes -----------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Public typedef ------------------------------------------------------------*/

/* Public define -------------------------------------------------------------*/
/**
 * @brief USART3 default baud rate.
 */
#define USART3_DEFAULT_BAUDRATE 115200

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
extern UART_HandleTypeDef huart3;

/* Public function prototypes ------------------------------------------------*/

/**
 * @brief  Initializes the USART3 peripheral with default configuration.
 * @note   This function uses the default settings defined in this header file.
 * @retval None
 */
void MX_USART3_UART_Init(void);

#endif /* INC_USART_CONFIG_H_ */
