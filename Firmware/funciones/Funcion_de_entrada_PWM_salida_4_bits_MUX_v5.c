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

/* Private variables ---------------------------------------------------------*/
uint32_t num_grey;
int array[16];
float valores[10];
float temp = 0;
float Tem = 0;

void SystemClock_Config(void);

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

float convertir_temperatura(float num)
{
	if (num<150 && num>3){
		temp = num * 1.5432 + 20.3704;
	}
	else {
		temp = 25;
	}
	return temp;
}

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

float realizar_medicion()
{
	Duty = 0;
	duty_sum = 0;
	duty_prom = 0;
	contador = 0;
	while (contador < 11)
	{
		__disable_irq();
		duty_updated = 0;
		valores[contador-1] = ObtenerDuty();
		__enable_irq();

		if (duty_updated)
		{
			HAL_Delay(1);
			duty_sum += valores[contador-1];
			contador ++;
			duty_updated = 0;
		}
	}
	return duty_sum;
}

void cambiar_canal(void)
{
	uint8_t i = 0;
	while (i < 16)
	{
		uint32_t gray = grayCode(i);
		setPins(gray);
		HAL_Delay(1);
		//contador = 0;
		float tolerance = 0.1;
		duty_prom = realizar_medicion() / 10.0;
		Tem = convertir_temperatura(duty_prom);
		array[gray] = round(Tem / tolerance)*tolerance;
		i ++;
		if (i == 16)
		{
			i = 0;
		}
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
	
	cambiar_canal();
	
	while (1)
  {
	  HAL_Delay(100);
    
  }
}

