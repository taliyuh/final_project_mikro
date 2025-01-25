/**
  ******************************************************************************
  * @file     : fan_config.c
  * @author   : taliyuh bartlomiej.hryniewski@student.put.poznan.pl
  * @version  : 1.0.0
  * @date     : Jan 21, 2025
  * @brief    : Electric fan components configuration file
  *
  ******************************************************************************
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
