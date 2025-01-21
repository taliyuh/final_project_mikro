/*
 * fan_config.c
 *
 *  Created on: Jan 21, 2025
 *      Author: bartl
 */

/* Private includes ----------------------------------------------------------*/
#include "fan.h"
#include "tim.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/


FAN_PWM_Handle_TypeDef hfan = FAN_PWM_INIT_HANDLE(&htim3, TIM_CHANNEL_1, FAN_ON_HIGH);

/* Private function prototypes -----------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
