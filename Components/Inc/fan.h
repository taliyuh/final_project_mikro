/**
  ******************************************************************************
  * @file     : fan.h
  * @author   : taliyuh    bartlomiej.hryniewski@student.put.poznan.pl
  * @version  : 1.0.0
  * @date     : Jan 21, 2025
  * @brief    : Electric fan components driver
  *
  ******************************************************************************
  */

#ifndef INC_FAN_H_
#define INC_FAN_H_

/* Public includes -----------------------------------------------------------*/
#include "dio.h"
#include "pwm.h"

/* Public typedef ------------------------------------------------------------*/
typedef enum { FAN_ON_LOW, FAN_ON_HIGH } FAN_ActiveState_TypeDef;
typedef enum { FAN_OFF = 0, FAN_ON = 1} FAN_DIO_State_TypeDef;

/**
  * @brief Digital output (On/Off) FAN handle structure definition
  */
typedef struct {
  DIO_Handle_TypeDef Output;
  FAN_ActiveState_TypeDef ActiveState;
} FAN_DIO_Handle_TypeDef;

/**
  * @brief PWM output (0.-100.%) FAN handle structure definition
  */
typedef struct {
  PWM_Handle_TypeDef Output;
  FAN_ActiveState_TypeDef ActiveState;
} FAN_PWM_Handle_TypeDef;

/* Public define -------------------------------------------------------------*/

/* Public macro --------------------------------------------------------------*/
/**
  * @brief Digital output (On/Off) fan handle structure initialization
  * @param[in] USER_NAME    : Pin user label set up in CubeMX (.ioc file)
  * @param[in] ACTIVE_STATE : Active state (polarity) of FAN
  *       This parameter can be one of the following values:
  *            @arg FAN_ON_LOW  : Fan turns on if output state is 0
  *            @arg FAN_ON_HIGH : Fan turns on if output state is 1
  */
#define FAN_DIO_INIT_HANDLE(USER_NAME, ACTIVE_STATE)  \
  {                                                 \
    .Output = DIO_INIT_HANDLE(USER_NAME),             \
    .ActiveState = ACTIVE_STATE                       \
  }

/**
  * @brief @brief PWM output (0.-100.%) fan handle structure initialization
  * @param[in] TIMER        : TIM handle
  *       This parameter can be one of the following values:
  *            @arg &htim1  : TIM1 (Advanced control timer, 6 channels)
  *            @arg &htim2  : TIM2 (General purpose timer, 4 channels)
  *            @arg &htim3  : TIM3 (General purpose timer, 4 channels)
  *            @arg &htim4  : TIM4 (General purpose timer, 4 channels)
  *            @arg &htim5  : TIM5 (General purpose timer, 4 channels)
  *            @arg &htim8  : TIM8 (Advanced control timer, 6 channels)
  *            @arg &htim9  : TIM9 (General purpose timer, 2 channels)
  *            @arg &htim10 : TIM10 (General purpose timer, 1 channel)
  *            @arg &htim11 : TIM11 (General purpose timer, 1 channel)
  *            @arg &htim12 : TIM12 (General purpose timer, 2 channels)
  *            @arg &htim13 : TIM13 (General purpose timer, 1 channel)
  *            @arg &htim14 : TIM14 (General purpose timer, 1 channel)
  * @param[in] CHANNEL      : TIM Channel
  *       This parameter can be one of the following values:
  *            @arg TIM_CHANNEL_1: TIM Channel 1 selected
  *            @arg TIM_CHANNEL_2: TIM Channel 2 selected
  *            @arg TIM_CHANNEL_3: TIM Channel 3 selected
  *            @arg TIM_CHANNEL_4: TIM Channel 4 selected
  *            @arg TIM_CHANNEL_5: TIM Channel 5 selected
  *            @arg TIM_CHANNEL_6: TIM Channel 6 selected
  * @param[in] ACTIVE_STATE : Active state (polarity) of FAN
  *       This parameter can be one of the following values:
  *            @arg FAN_ON_LOW  : Fan turns on if output state is 0
  *            @arg FAN_ON_HIGH : Fan turns on if output state is 1
  */
#define FAN_PWM_INIT_HANDLE(TIMER, CHANNEL, ACTIVE_STATE)  \
  {                                                      \
    .Output = PWM_INIT_HANDLE((TIMER), CHANNEL),         \
    .ActiveState = ACTIVE_STATE                          \
  }

/* Public variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/**
  * @brief Turns fan on
  * @param[in] hfan   : Fan DIO handler
  * @retval None
  */
void FAN_DIO_On(const FAN_DIO_Handle_TypeDef* hfan);

/**
  * @brief Turns fan off
  * @param[in] hfan   : Fan DIO handler
  * @retval None
  */
void FAN_DIO_Off(const FAN_DIO_Handle_TypeDef* hfan);

/**
  * @brief Toggles fan state
  * @param[in] hfan   : Fan DIO handler
  * @retval None
  */
void FAN_DIO_Toggle(const FAN_DIO_Handle_TypeDef* hfan);

/**
  * @brief Writes given fan state
  * @param[in] hfan   : Fan DIO handler
  * @param[in] state  : Fan state (FAN_OFF or FAN_ON)
  * @retval None
  */
void FAN_DIO_Write(const FAN_DIO_Handle_TypeDef* hfan, FAN_DIO_State_TypeDef state);

/**
  * @brief Reads fan state
  * @param[in] hfan   : Fan DIO handler
  * @retval Fan state (FAN_OFF or FAN_ON)
  */
FAN_DIO_State_TypeDef FAN_DIO_Read(const FAN_DIO_Handle_TypeDef* hfan);

/**
  * @brief Initialize PWM fan control
  * @param[in/out] hfan   : Fan PWM handler
  * @retval None
  */
void FAN_PWM_Init(FAN_PWM_Handle_TypeDef* hfan);

/**
  * @brief Write PWM duty cycle
  * @param[in/out] hfan   : Fan PWM handler
  * @param[in]     duty   : PWM duty cycle in percents (0. - 100.)
  * @retval None
  */
void FAN_PWM_WriteDuty(FAN_PWM_Handle_TypeDef* hfan, float duty);

/**
  * @brief Set PWM duty cycle
  * @param[in] hfan   : Fan PWM handler
  * @retval PWM duty cycle in percents (0. - 100.)
  */
float FAN_PWM_ReadDuty(const FAN_PWM_Handle_TypeDef* hfan);

#endif /* INC_FAN_H_ */
