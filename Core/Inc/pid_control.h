/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : pid_control.h
  * @author   : taliyuh    bartlomiej.hryniewski@student.put.poznan.pl
  * @version  : 1.0.0
  * @date     : Jan 21, 2025
  * @brief    : Header file for PID control functions.
  ******************************************************************************
*/
/* USER CODE END Header */

#ifndef PID_CONTROL_H
#define PID_CONTROL_H

#include <stdint.h>

/**
 * @brief PID parameters structure.
 */
typedef struct {
    float kp;           /**< Proportional gain */
    float ki;           /**< Integral gain */
    float kd;           /**< Derivative gain */
    float integral_max; /**< Maximum integral value */
    float integral_min; /**< Minimum integral value */
} PID_Params_t;

/**
 * @brief PID controller context structure.
 */
typedef struct {
    float target_temperature;
    float current_temperature;
    float error;
    float integral;
    float derivative;
    float previous_error;
    float output;
    float output_saturated;
} PID_Control_t;

/**
 * @brief Initializes the PID controller.
 */
void PID_Init(void);

/**
 * @brief Updates the PID controller with the current temperature.
 * @param current_temperature The current measured temperature.
 */
void PID_Update(float current_temperature);

/**
 * @brief Calculates the PID output.
 * @return The calculated PID output.
 */
float PID_Calculate(void);

extern PID_Params_t pid_params;
extern PID_Control_t pid_controller;

#endif /* PID_CONTROL_H */
