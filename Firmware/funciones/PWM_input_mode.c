/*
Función que mide el ciclo de trabajo y la frecuencia de una señal PWM externa.

Primero se tiene que establecer los parámetros del programa y del clock:

1. En Pinout y configuration, en el apartado de System Core, luego en RCC de Reset and clock control, se habilita el HSE o high speed external 
clock, con la opción Crystal Resonator. 
2. Luego en Clock Configuration se deja el valor de imput frecuency HSE default, pero se usa el maximo valor permitido en HCLK (en mi caso los 
84MHz) que alimanetarán a los 2 ciclos de timer siendo unos timer alimentados con 84MHz y otros con 42MHz. Se ajuntan en automatico los valores
de los preescaler y de la PLL.
3. Luego volviendo a Pinout y configuration, en el apartado de Timers y TIM2, se habilita el timer 2 (tiene menos frecuencia de reloj 42MHz, 
pero su timer tiene más bits 2^16), se ajusta el clock source con Internal clock, se ajusta el canal 1 en input capture, en el apartado de 
Parameter settings, para Prescaler (PSC) se deja un valor de 0 (que en realidad es 1) para que la frecuencia del timer sea de 42MHz, luego 
el counter period ARR, se deja en el valor máximo permitido para que pueda contar desde el 0 hasta 2^16.
4. Trigger y Slave mode desabilitado. En el Input Capture Channel 1, la polaridad se deja en Both Edges, con IC Direct, sin Prescaler Division 
Ratio y con un Input Filter de 8, para que filter la señal ****
5. En el apartado de NVIC settings, se habilita el TIM2 global interrupt.
6. Luego en el apartado de System Core, luego GPIO, en la sección TIM, se habilita los pines GPIO de los canales correspondientes que se han 
iniciado (podrian ser 2 para una señal pwm, pero considerando que pueden haber muchas, se quiere usar un canal y un pin para cada pwm), en la 
seccion Maximum output speed se elije High y asi se habilita el pin para Input de la señal externa.
7. Ahora se procede a escribir el código
*/
/*  Se pueden definir variables fuera del void SystemClock_Config(void); para que sean globales y sin valor cambiable en el loop. Tambien se 
pueden definir adentro de la función.
	Después del void SystemClock_Config(void); se define la función a implementar, aqui va todo lo relacionado a los calculos y a las 
interrupciones y los conteos.
	Dentro del int main(void) se debe inicializar los timer a usar con HAL_TIM_IC_Start_IT(&htimx, TIM_CHANNEL_y) con el segundo canal sin la 
	interrupción, asi que sin el IT; Luego dentro del while (1) se puede agregar un HAL_Delay(1); para tener un breakpoint dentro de while (1).
*/
uint32_t capture1_rising, capture1_falling, capture2;
uint32_t period, pulse_width;
float frequency, duty_cycle;
int32_t F_CPU = 42000000;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) 
{
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        // Lógica para determinar qué flanco se capturó
        if (capture1_rising == 0) {
            capture1_rising = capture_value; // Primer flanco ascendente
        } else if (capture1_falling == 0) {
            capture1_falling = capture_value; // Primer flanco descendente
            pulse_width = capture1_falling - capture1_rising; // Ancho del pulso
        } else {
            capture2 = capture_value; // Segundo flanco ascendente
            period = capture2 - capture1_rising; // Período de la señal
            frequency = 2*(float)F_CPU / period;
            duty_cycle = (float)pulse_width / period * 100;

            // Hacer algo con los valores calculados
            // ...

            // Reiniciar las variables para la siguiente medición
            __HAL_TIM_SET_COUNTER(&htim2, 0);
            capture1_rising = 0;
            capture1_falling = 0;
            capture2 = 0;
        }
    }
}
/* Hasta acá todo bien con la frecuencia y mas o menos con el ciclo de trabajo, porque la frecuencia está bien calculada, pero el ciclo de 
trabajo a veces da el valor de D y otra veces el valor de 1-D, lo que ocaciona error en la lectura. Por lo menos puede leer todos los rangos 
incluyendo el deseado de 3%-85%, por lo que es un punto a favor. El ciclo de trabajo no es que esté mal pero hay problemas con la captura
que depende del clock del uC y de su conteo:

	Caso Normal: en un canto de subida del clock, se captura el Rising de la señal PWM, luego se captura el Falling de la señal, con esto se 
	calcula el duty_cycle y luego con la captura del segundo Rising se calcula la frecuencia usando las 3 variables, una capture1_rising, 
	luego capture1_falling y luego capture2 que almacena el siguiente ciclo.
	
	Caso Desfasado: no se alcanza a capturar el Rising entre 2 ciclos del clock y el capture1_falling lee el Falling de la señal, luego el 
	capture 2 lee el Rising de la señal y con eso calcula el DC y luego el capture1_rising lee la nueva Rising, lo cual calcula mal el 
	ciclo de trabajo.
	
Soluciones para arreglar esto:	
*/

// Solucion Final:

// 1. Habilitar en el timer 2 la opción Combined Channels, luego la oppción PWM Input on CH1. Luego en la configuración de Parameters
// setings, Setear el PSC como 0 y el máximo de ARR. 
// 2. Luego en la seccion de PWM Input CH1: el Input triggger TI1FP1, con Slave Mode Controller en Reset Mode.
// 3. En Parameters for Channel 1: Polarity = Rising Edge, IC Selec = Direct, Prescaler = No division y Input Filter = 0.
// 4. En Parameters for Channel 2: Polarity = Falling Edge, IC Selec = Indirect, Prescaler = No division y Input Filter = 0.
// 5. Se habilita la interrupción global y el puerto de GPIO correspondiente. 

#include "main.h"
#include "tim.h"
#include "gpio.h"

uint32_t ICValue;
float Frequency;
float Duty = 0;

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

			Frequency = 2*42000000/(float)ICValue;
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
	
	 while (1)
	{
    /* USER CODE END WHILE */
	  HAL_Delay(50);
    /* USER CODE BEGIN 3 */
	}
}



// Variables globales (ajustar según tu proyecto)
uint32_t capture_rising = 0;
uint32_t capture_falling = 0;
float duty_cycle = 0.0;
const float PERIOD_9KHZ = 111.1; // Período de una señal de 9kHz en microsegundos

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) { // Ajustar al canal que estés usando
        uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        if (capture_rising == 0) {
            // Primer flanco de subida
            capture_rising = capture_value;
        } else {
            // Segundo flanco (se asume que es de bajada)
            capture_falling = capture_value;

            // Calcular el ancho de pulso y el ciclo de trabajo
            if (capture_falling > capture_rising) {
                uint32_t pulse_width = capture_falling - capture_rising;
                duty_cycle = (float)pulse_width / PERIOD_9KHZ * 100;
                // Hacer algo con el valor del ciclo de trabajo
                printf("Ciclo de trabajo: %.2f%%\n", duty_cycle);
            }

            // Reiniciar las variables para la siguiente medición
            capture_rising = 0;
            capture_falling = 0;
        }
    }
}

//Gemini
#define THRESHOLD 10 // Umbral de cambio
uint32_t previous_capture1, previous_capture2, current_capture;
uint32_t period, pulse_width;
bool is_rising = false;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        // Lógica para determinar qué flanco se capturó
        if (previous_capture1 == 0) {
            previous_capture1 = current_capture;
        } else {
            // Verificar si ha ocurrido un cambio de estado
            if (abs(current_capture - previous_capture1) > THRESHOLD) {
                if (current_capture > previous_capture1) {
                    is_rising = true;
                } else {
                    is_rising = false;
                }

                // Calcular período y ancho de pulso (lógica similar a la versión anterior)
                // ...

                // Actualizar historial
                previous_capture2 = previous_capture1;
                previous_capture1 = current_capture;
            } else {
                // Descartar la captura ya que no representa un cambio significativo
                previous_capture2 = previous_capture1;
                previous_capture1 = current_capture;
            }
        }
    }
}

//Copilot
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        // Lógica para determinar qué flanco se capturó
        if (capture1_rising == 0) {
            capture1_rising = capture_value; // Primer flanco ascendente
        } else if (capture1_falling == 0) {
            capture1_falling = capture_value; // Primer flanco descendente
            pulse_width = capture1_falling - capture1_rising; // Ancho del pulso
        } else if (capture2 == 0) {
            capture2 = capture_value; // Segundo flanco ascendente
            period = capture2 - capture1_rising; // Período de la señal
            frequency = 2 * (float)F_CPU / period;
            duty_cycle = (float)pulse_width / period * 100;

            // Hacer algo con los valores calculados
            // ...

            // Reiniciar las variables para la siguiente medición
            __HAL_TIM_SET_COUNTER(&htim2, 0);
            capture1_rising = 0;
            capture1_falling = 0;
            capture2 = 0;
        }
    }
}
//Para asegurar que siempre se capturen los flancos en el orden correcto, puedes agregar una verificación adicional para asegurarte de 
//que el flanco descendente se capture después del flanco ascendente:

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        // Lógica para determinar qué flanco se capturó
        if (capture1_rising == 0) {
            capture1_rising = capture_value; // Primer flanco ascendente
        } else if (capture1_falling == 0 && capture_value > capture1_rising) {
            capture1_falling = capture_value; // Primer flanco descendente
            pulse_width = capture1_falling - capture1_rising; // Ancho del pulso
        } else if (capture2 == 0 && capture_value > capture1_falling) {
            capture2 = capture_value; // Segundo flanco ascendente
            period = capture2 - capture1_rising; // Período de la señal
            frequency = 2 * (float)F_CPU / period;
            duty_cycle = (float)pulse_width / period * 100;

            // Hacer algo con los valores calculados
            // ...

            // Reiniciar las variables para la siguiente medición
            __HAL_TIM_SET_COUNTER(&htim2, 0);
            capture1_rising = 0;
            capture1_falling = 0;
            capture2 = 0;
        }
    }
}

//ChatGPT
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    static uint8_t last_edge = 0; // 0 = ningún flanco, 1 = ascendente, 2 = descendente
    static uint32_t rising_edge_time = 0;
    static uint32_t falling_edge_time = 0;

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        // Determinar el tipo de flanco actual
        if (last_edge == 0 || last_edge == 2) { // Flanco ascendente
            rising_edge_time = capture_value;
            last_edge = 1;
        } else if (last_edge == 1) { // Flanco descendente
            falling_edge_time = capture_value;
            last_edge = 2;

            // Calcular el ancho del pulso considerando el posible desbordamiento
            if (falling_edge_time >= rising_edge_time) {
                pulse_width = falling_edge_time - rising_edge_time;
            } else { // Manejar desbordamiento del temporizador
                pulse_width = (htim->Instance->ARR - rising_edge_time) + falling_edge_time + 1;
            }
        }

        // Calcular el período completo si se detecta otro flanco ascendente
        if (last_edge == 1 && falling_edge_time != 0) {
            if (rising_edge_time >= falling_edge_time) {
                period = rising_edge_time - falling_edge_time;
            } else { // Manejar desbordamiento del temporizador
                period = (htim->Instance->ARR - falling_edge_time) + rising_edge_time + 1;
            }

            // Calcular frecuencia y ciclo de trabajo
            frequency = (float)F_CPU / period;
            duty_cycle = (float)pulse_width / period * 100.0f;

            // Reiniciar para la siguiente medición
            __HAL_TIM_SET_COUNTER(htim, 0);
            rising_edge_time = 0;
            falling_edge_time = 0;
            last_edge = 0;

            // Aquí puedes hacer algo con los valores calculados
        }
    }
}

	
	