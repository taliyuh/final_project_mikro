/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart_config.c
  * @author  taliyuh    bartlomiej.hryniewski@student.put.poznan.pl
  * @brief   This file provides code for the configuration
  *          of the USART3 instance.
  * @version  : 1.0.0
  * @date     : Jan 21, 2025
  * @brief    : Electric fan components driver
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usart_config.h"
#include "main.h" // Assuming you have GPIO and other definitions there

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/**
 * @brief USART3 default baud rate.
 */
#define USART3_DEFAULT_BAUDRATE 115200

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
/**
 * @brief USART3 handle.
 */
UART_HandleTypeDef huart3;

/* Private function prototypes -----------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initializes the USART3 peripheral.
 * @retval None
 */
void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = USART3_DEFAULT_BAUDRATE;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}
