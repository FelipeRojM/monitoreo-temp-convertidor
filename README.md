# monitoreo-temp-convertidor
# Sistema de Medición de Temperatura para Convertidores

![Hardware](https://img.shields.io/badge/Hardware-Altium_Designer-blue?style=for-the-badge)
![Firmware](https://img.shields.io/badge/Firmware-Microcontroladores-orange?style=for-the-badge)

## 📌 Descripción General
Este repositorio documenta el diseño de hardware y el desarrollo de firmware para un sistema de medición y monitoreo de temperatura en convertidores de potencia. Este proyecto fue desarrollado como parte de mis funciones de ingeniería durante mi práctica en el AC3E para los convertidores utilizados en los racks de su laboratorio.

El sistema permite adquirir variables térmicas críticas y procesarlas a través de un microcontrolador, facilitando el mantenimiento preventivo y asegurando la correcta disipación térmica de los equipos.


## ⚙️ Características del Proyecto

### 💻 Firmware (Microcontrolador)
* **Microcontrolador:** *(Ej: STM32, ESP32, PIC, etc.)*
* **Lenguaje:** C / C++ 
* **Protocolos de Comunicación:** *(Ej: I2C/SPI para leer los sensores digitales, o ADC para termistores)*
* **Lógica Principal:** Lectura continua de sensores térmicos, filtrado de señales y procesamiento de datos.

### 🔌 Hardware (Altium Designer)
* **Diseño Esquemático:** Circuitería de acondicionamiento de señales para los sensores de temperatura.
* **PCB Layout:** Ruteo estratégico para aislar la etapa de medición analógica del ruido electromagnético de los convertidores.
* **Componentes Clave:** 

## 📂 Estructura del Repositorio
* `/Firmware`: Contiene el código fuente y las rutinas de lectura de los sensores.
* `/Hardware`: Contiene el proyecto de Altium Designer con los esquemáticos y diseño de la PCB.
* `/Docs`: Esquemáticos exportados en PDF para revisión rápida sin software EDA.

## 👨‍💻 Autor
**Felipe Rojas**  
*Ingeniero Civil Electrónico*  
[Perfil de LinkedIn](https://www.linkedin.com/in/felipe-rojas-monrroy)