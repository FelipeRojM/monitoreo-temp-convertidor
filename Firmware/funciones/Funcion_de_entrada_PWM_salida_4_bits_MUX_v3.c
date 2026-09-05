#include "main.h"

// Variables globales
volatile float Duty = 0;         // Última medición del duty cycle
volatile float duty_sum = 0;     // Suma acumulada del duty cycle
volatile int mediciones_contador = 0;  // Contador de mediciones realizadas
volatile int mediciones_completas = 0; // Bandera para saber si se completaron 10 mediciones

// Interrupción del temporizador
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        // Leer los valores capturados
        uint32_t ICValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        if (ICValue != 0) {
            Duty = ((float)HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) * 100) / (float)ICValue;

            // Acumular la medición
            duty_sum += Duty;
            mediciones_contador++;

            // Revisar si ya se completaron las 10 mediciones
            if (mediciones_contador >= 10) {
                mediciones_completas = 1;  // Señalar que las mediciones están completas
                HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1);  // Detener el temporizador para evitar nuevas interrupciones
            }
        }
    }
}

// Función para inicializar las mediciones (bloqueante hasta completar)
float inicializar_mediciones(TIM_HandleTypeDef *htim) {
    // Reiniciar variables
    duty_sum = 0;
    mediciones_contador = 0;
    mediciones_completas = 0;

    // Iniciar el temporizador con interrupciones
    HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
    HAL_TIM_IC_Start(htim, TIM_CHANNEL_2);

    // Esperar hasta que las mediciones se completen
    while (mediciones_completas == 0) {
        // Esperar (puedes agregar algún timeout aquí si es necesario)
    }

    // Calcular el promedio de las mediciones
    float duty_prom = duty_sum / 10.0;

    // Reiniciar bandera
    mediciones_completas = 0;

    return duty_prom;
}

// Función principal
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    uint32_t array[16];  // Almacenar los valores medidos

    for (int i = 0; i < 16; i++) {
        // Generar el código Gray
        uint32_t gray = grayCode(i);
        setPins(gray);

        // Realizar las mediciones (espera hasta que se completen las 10 mediciones)
        array[gray] = (int)round(inicializar_mediciones(&htim2));

        HAL_Delay(200);  // Pequeño retraso antes del próximo cambio de Gray (si es necesario)
    }

    while (1) {
        // Loop principal
    }
}














#include "main.h"

// Variables globales
volatile float Duty = 0;             // Última medición del duty cycle
volatile int mediciones_contador = 0;  // Contador de mediciones realizadas
volatile int mediciones_completas = 0; // Bandera: 1 si ya se completaron las 10 mediciones
float duty_sum = 0;                   // Acumulador de mediciones

// Función para inicializar las mediciones
void inicializar_mediciones(void) {
    // Reiniciar variables
    mediciones_contador = 0;
    duty_sum = 0;
    mediciones_completas = 0;

    // Iniciar los canales del timer
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_2);
}

// Función para procesar las mediciones (llamada desde el `main`)
float esperar_mediciones_completas(void) {
    // Esperar hasta que las mediciones estén completas
    while (mediciones_completas == 0) {
        // Bloqueo hasta que se complete el proceso
    }

    // Detener los canales del timer para evitar más interrupciones
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_2);

    // Calcular el promedio
    return duty_sum / 10.0;
}

// La función `HAL_TIM_IC_CaptureCallback` se mantiene sin cambios
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {  // Si la interrupción es del canal 1
        uint32_t ICValue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        if (ICValue != 0) {
            // Calcular el Duty Cycle
            Duty = ((float)HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) * 100) / (float)ICValue;

            // Acumular el Duty Cycle y aumentar el contador
            duty_sum += Duty;
            mediciones_contador++;

            // Verificar si ya se completaron las 10 mediciones
            if (mediciones_contador >= 10) {
                mediciones_completas = 1;  // Indicar que las mediciones están completas
            }
        }
    }
}

// Función principal
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    uint32_t array[16];  // Almacena los valores medidos

    for (int i = 0; i < 16; i++) {
        // Generar el código Gray
        uint32_t gray = grayCode(i);
        setPins(gray);

        // Inicializar las mediciones
        inicializar_mediciones();

        // Esperar hasta completar las 10 mediciones
        array[gray] = (int)round(esperar_mediciones_completas());

        HAL_Delay(200);  // Pequeño retraso antes del próximo cambio de Gray (si es necesario)
    }

    while (1) {
        // Loop principal
    }
}








/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include <math.h>

uint32_t ICValue;
float Duty = 0;
int duty_updated = 0;

// Variables globales
float duty_sum = 0;       // Suma acumulativa de los duty cycles
float duty_prom;  		  // Promedio calculado
int contador = 0;
int mediciones_completas = 0;

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
		}

	}
}
float ObtenerDuty(void)
{
	duty_updated = 1;
    return Duty;
}

void iniciar_mediciones() {
	Duty = 0;
	duty_sum = 0;
	contador = 0;
	mediciones_completas = 0;
}

void realizar_medicion() {
	contador++;
    duty_sum += ObtenerDuty();
    if (contador == 10) {
        duty_prom = duty_sum / 10.0;
        mediciones_completas = 1;
    }
}

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

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_2);

  /* USER CODE END 2 */
  for (int i = 0; i < 16; i++) {
       uint32_t gray = grayCode(i);
       HAL_Delay(10);
       setPins(gray);
       iniciar_mediciones();
       while (!mediciones_completas){
    	   realizar_medicion();
    	   HAL_Delay(10);
       }
       array[gray] = (int) round(duty_prom);
  }
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}









