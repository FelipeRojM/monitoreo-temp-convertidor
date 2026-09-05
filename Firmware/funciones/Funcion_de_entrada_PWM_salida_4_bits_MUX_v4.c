/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include <math.h>

/* Private define ------------------------------------------------------------*/
uint32_t ICValue;
float Duty = 0;
int duty_updated = 0;

// Variables globales
float duty_sum = 0;       // Suma acumulativa de los duty cycles
float duty_prom;  		  // Promedio calculado
int contador = 0;
float valores[10];

/* Private variables ---------------------------------------------------------*/
uint32_t num_grey;
int array[16];

void SystemClock_Config(void);

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
			//duty_updated = 1;
		}

	}
}
float ObtenerDuty(void)
{
	duty_updated = 1;
    return Duty;
}

//Funciones 
// Función para generar el código Gray de un número
uint32_t grayCode(uint32_t num) {
	num_grey = num ^(num>>1);
    return num_grey;
}

// Función para configurar los pines PB0 y PB1
void setPins(uint32_t gray) {
    if (gray & 0x08) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // PB0 en alto
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // PB0 en bajo
    }

    if (gray & 0x04) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); // PB1 en alto
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); // PB1 en bajo
    }
	if (gray & 0x02) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET); // PB2 en alto
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // PB2 en bajo
    }

    if (gray & 0x01) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // PB10 en alto
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); // PB10 en bajo
    }
}

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_TIM2_Init();
	
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_2);
	
	int i = 0;
	while (i < 16)
	{
		uint32_t gray = grayCode(i);  // Crear secuencia de Grey
		setPins(gray);     // Setear los pines necesarios de salida para el entrada del mux
		HAL_Delay(1);
		Duty = 0;	 // Resetear variables
		duty_sum = 0;
		duty_prom = 0;
		contador = 0;
		while (contador < 11)
		{
			__disable_irq();   	// Deshabilitar interrupciones globalmente
			duty_updated = 0;
			valores[contador-1] = ObtenerDuty(); // Código para procesar el ciclo de trabajo
			__enable_irq();    // Habilitar interrupciones nuevamente

			if (duty_updated)
			{
				HAL_Delay(1);
				duty_sum += valores[contador-1];    // Guardar la suma de valores
				contador ++;
				duty_updated = 0;
			}
			//HAL_Delay(1);
		}
		//contador = 0;
		duty_prom = duty_sum / 10.0;   // Calcular el promedio
		array[gray] = (int) round(duty_prom);  // Guardar el promedio de cada canal
		//HAL_Delay(1);
		i ++;
	}


	while (1)
	{
		/* USER CODE END WHILE */
		HAL_Delay(100);
		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}





