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
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint32_t ICValue;
float Duty = 0;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

// Variables globales
float duty_sum = 0;       // Suma acumulativa de los duty cycles
float duty_prom;  // Promedio calculado
int contador = 0;
// Contador de mediciones completadas

/* USER CODE BEGIN PV */
uint32_t num_grey;
int array[4];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */


/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // If the interrupt is triggered by channel 1
	{
		// Read the IC value
		ICValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

		if (ICValue != 0)
		{
			// calculate the Duty Cycle
			Duty = ((float)HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) *100)/(float)ICValue;

		}

	}
}

float ObtenerDuty(void)
{
    return Duty;
}

float inicializar_mediciones() {
    // Realizar las 10 mediciones iniciales
    for (int i=0; i<10; i++) {
    	contador++;
    	duty_sum = duty_sum + ObtenerDuty();

    }
    if (contador == 10){
    	duty_prom = duty_sum/10.0;
    	contador=0;
    	duty_sum=0;
    }
    return duty_prom;
}

// Función para generar el código Gray de un número
uint32_t grayCode(uint32_t num) {
	num_grey = num ^(num>>1);
    return num_grey;
}

// Función para configurar los pines PB0 y PB1
void setPins(uint32_t gray) {
    if (gray & 0x02) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // PB0 en alto
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // PB0 en bajo
    }

    if (gray & 0x01) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); // PB1 en alto
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); // PB1 en bajo
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // Inicia la captura de entrada
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_2);

  //HAL_TIM_Base_Init(&htim2);
  /* USER CODE END 2 */
  for (int i = 0; i < 4; i++) {
      uint32_t gray = grayCode(i);
      HAL_Delay(10);
      setPins(gray);
      array[gray] = (int) round(inicializar_mediciones());
      HAL_Delay(10); // Esperar 1 segundo entre cambios
      }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  HAL_Delay(100);
	  //inicializar_mediciones();

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
