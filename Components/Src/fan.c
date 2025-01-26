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

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
/**
  * @brief Turns fan on
  * @param[in] hfan   : Fan DIO handler
  * @retval None
  */
void FAN_DIO_On(const FAN_DIO_Handle_TypeDef* hfan)
{
  FAN_DIO_Write(hfan, FAN_ON);
}

/**
  * @brief Turns fan off
  * @param[in] hfan   : Fan DIO handler
  * @retval None
  */
void FAN_DIO_Off(const FAN_DIO_Handle_TypeDef* hfan)
{
  FAN_DIO_Write(hfan, FAN_OFF);
}

/**
  * @brief Toggles fan state
  * @param[in] hfan   : Fan DIO handler
  * @retval None
  */
void FAN_DIO_Toggle(const FAN_DIO_Handle_TypeDef* hfan)
{
  DIO_Toggle(&(hfan->Output));
}

/**
  * @brief Writes given fan state
  * @param[in] hfan   : Fan DIO handler
  * @param[in] state  : Fan state (FAN_OFF or FAN_ON)
  * @retval None
  */
void FAN_DIO_Write(const FAN_DIO_Handle_TypeDef* hfan, FAN_DIO_State_TypeDef state)
{
  DIO_Write(&(hfan->Output), (hfan->ActiveState == FAN_ON_HIGH) ? state : !state);
}

/**
  * @brief Reads fan state
  * @param[in] hfan   : Fan GPIO handler
  * @retval Fan state (FAN_OFF or FAN_ON)
  */
FAN_DIO_State_TypeDef FAN_DIO_Read(const FAN_DIO_Handle_TypeDef* hfan)
{
  _Bool state = DIO_Read(&(hfan->Output));
  return (hfan->ActiveState == FAN_ON_HIGH) ? state : !state;
}

/**
  * @brief Initialize PWM fan control
  * @param[in] hfan   : Fan PWM handler
  * @retval None
  */
void FAN_PWM_Init(FAN_PWM_Handle_TypeDef* hfan)
{
  hfan->Output.Duty = (hfan->ActiveState == FAN_ON_HIGH) ? (hfan->Output.Duty) : (100.0f - hfan->Output.Duty);
  PWM_Init(&(hfan->Output));
}

/**
  * @brief Write PWM duty cycle
  * @param[in/out] hfan   : Fan PWM handler
  * @param[in]     duty   : PWM duty cycle in percents (0. - 100.)
  * @retval None
  */
void FAN_PWM_WriteDuty(FAN_PWM_Handle_TypeDef* hfan, float duty)
{
  const float minDuty = 60.0f; // Minimum duty cycle for the fan to start

  // Clamp the duty cycle to the valid range [0.0, 100.0]
  if (duty < 0.0f) {
    duty = 0.0f;
  } else if (duty > 99.0f) {
    duty = 99.0f;
  }

  // Map the duty cycle to the new range [minDuty, 100.0]
  if (duty > 0.0f) {
      duty = minDuty + (duty * (100.0f - minDuty) / 100.0f);
  }

  hfan->Output.Duty = (hfan->ActiveState == FAN_ON_HIGH) ? (duty) : (100.0f - duty);
  PWM_WriteDuty(&(hfan->Output), hfan->Output.Duty);
}

/**
  * @brief Set PWM duty cycle
  * @param[in] hfan   : Fan PWM handler
  * @retval PWM duty cycle in percents (0. - 100.)
  */
float FAN_PWM_ReadDuty(const FAN_PWM_Handle_TypeDef* hfan)
{
  return (hfan->ActiveState == FAN_ON_HIGH) ? (hfan->Output.Duty) : (100.0f - hfan->Output.Duty);
}
