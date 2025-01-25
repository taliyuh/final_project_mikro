/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file     : pid_control.c
  * @author   : taliyuh    bartlomiej.hryniewski@student.put.poznan.pl
  * @version  : 1.0.0
  * @date     : Jan 21, 2025
  * @brief    : This file provides functions for PID control.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "pid_control.h"
#include <math.h>

// Define the global PID parameters
PID_Params_t pid_params = {
    .kp = 7.0f,
    .ki = 0.25f,
    .kd = 250.0f,
    .integral_max = 500.0f,
    .integral_min = -500.0f
};

// Define the global PID controller
PID_Control_t pid_controller;

/**
 * @brief Initializes the PID controller.
 */
void PID_Init(void) {
    pid_controller.target_temperature = 0.0f;
    pid_controller.current_temperature = 0.0f;
    pid_controller.error = 0.0f;
    pid_controller.integral = 0.0f;
    pid_controller.derivative = 0.0f;
    pid_controller.previous_error = 0.0f;
    pid_controller.output = 0.0f;
    pid_controller.output_saturated = 0.0f;
}

/**
 * @brief Updates the PID controller with the current temperature.
 * @param current_temperature The current measured temperature.
 */
void PID_Update(float current_temperature) {
    pid_controller.current_temperature = current_temperature;
    pid_controller.error = pid_controller.target_temperature - current_temperature;
}

/**
 * @brief Calculates the PID output.
 * @return The calculated PID output.
 */
float PID_Calculate(void) {
    // Derivative Filter Parameters
    const float derivative_filter_constant = 0.9f;
    static float previous_derivative = 0.0f;
    const float back_calculation_gain = 0.1f;

    // Integral term with anti-windup
    pid_controller.integral += pid_controller.error;
    if (pid_controller.output_saturated != pid_controller.output) {
        pid_controller.integral -= back_calculation_gain * (pid_controller.output_saturated - pid_controller.output);
    }
    pid_controller.integral = fminf(fmaxf(pid_controller.integral, pid_params.integral_min), pid_params.integral_max);

    // Derivative term with filtering
    pid_controller.derivative = (pid_controller.error - pid_controller.previous_error);
    pid_controller.derivative = derivative_filter_constant * previous_derivative + (1.0f - derivative_filter_constant) * pid_controller.derivative;

    // Calculate PID output
    pid_controller.output = (pid_params.kp * pid_controller.error) + (pid_params.ki * pid_controller.integral) + (pid_params.kd * pid_controller.derivative);

    // Update previous values
    pid_controller.previous_error = pid_controller.error;
    previous_derivative = pid_controller.derivative;

    return pid_controller.output;
}
