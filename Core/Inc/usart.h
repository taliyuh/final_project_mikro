/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart_config.h" // Include the new configuration header

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

/**
  * @brief  UART MSP Initialization.
  *         This function configures the hardware resources used in this example:
  *         - Peripheral's clock enable
  *         - Peripheral's GPIO Configuration
  *         - NVIC configuration for UART interrupt request enable
  * @param  huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart);

/**
  * @brief  UART MSP De-Initialization.
  *         This function freeze the hardware resources used in this example:
  *         - Disable the Peripheral's clock
  *         - Revert GPIO configuration to their default state
  *         - Disable NVIC for UART interrupt request
  * @param  huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
