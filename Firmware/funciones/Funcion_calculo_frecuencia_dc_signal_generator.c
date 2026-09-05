/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float frequency;
float duty_cycle;

static uint32_t last_capture = 0;
uint32_t capture = 0;
float period = 0;
float high_time = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* funcion 1

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    period = (float)(capture - last_capture);
    last_capture = capture;

    // Calcula la frecuencia
    frequency = 1000000.0f / period; // 1 MHz / período en µs

    // Captura el valor del canal 2 para calcular el ciclo de trabajo
    high_time = (float)HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    duty_cycle = (high_time * 100.0f)/period;

    // Imprime los valores en la consola
    //printf("Frequency: %.2f Hz\n", frequency);
    //printf("Duty Cycle: %.2f %%\n", duty_cycle);
}
*/

// funcion 2
 void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
 {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        period = (float)(capture - last_capture);
        last_capture = capture;

        // Calcula la frecuencia
        frequency = 1000000.0f / period; // 1 MHz / período en µs
    }

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        high_time = (float)HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

        // Calcula el ciclo de trabajo
        duty_cycle = (high_time * 100.0f) / period;
    }
}

// funcion 3
 void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
 {
     static uint32_t last_capture = 0;
     static uint32_t high_time = 0;
     uint32_t capture;
     float period;

     if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
         capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

         if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1OF)) {
             // Overflow flag is set, handle overflow
             __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1OF);
             return;
         }

         if (last_capture == 0) {
             last_capture = capture;
             return;
         }

         if (capture > last_capture) {
             period = (float)(capture - last_capture);
         } else {
             period = (float)((0xFFFF - last_capture) + capture + 1);
         }

         if (__HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1) == capture) {
             high_time = period;
         } else {
             frequency = 1000000.0f / period; // 1 MHz / período en µs
             duty_cycle = (high_time * 100.0f) / period;
         }

         last_capture = capture;
     }
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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /*TIM1->CCR1 = 890;
  //__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 55);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
*/

  // Inicia la captura de entrada
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  HAL_Delay(1000);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */