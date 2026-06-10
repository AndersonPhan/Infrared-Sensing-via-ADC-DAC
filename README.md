# Infrared-Sensing
Infrared sensing system using a STM32 Microcontroller with ADC/DAC triggering via Timer 4, object. Object distance is detected with a photodiode, which activates a buzzer, LED, and tera term output when the treshold is met. This project is meant to be run in Keil uVision5.

## Introduction
This project implements a proximity sensing system using an STM32 development board with an infrared LED and photodiode sensor pair. The system detects the presence of an object based on reflected infrared light and triggers both visual and audio outputs when a defined threshold is reached.<br>

An infrared LED (5 mm) is used as the emitter, while a photodiode (5 mm) serves as the receiver to measure reflected signal intensity. The analog signal is processed through the STM32’s ADC to determine proximity levels. When an object comes within a specified threshold distance, the system activates a 440 Hz tone on a buzzer and turns on the onboard green LED (PA5). <br>

Timer 4 Channel 1 is configured to trigger both ADC and DAC conversions, ensuring synchronized signal sampling and output generation. The ADC is used to read voltage levels corresponding to sensor distance, while the DAC assists in signal generation for output control. <br>

The project is structured into four main tasks: configuring Timer 4 to trigger ADC and DAC, implementing ADC-based distance sensing, generating a 440 Hz buzzer output when the threshold is exceeded, and displaying real-time sensor data through Tera Term. <br>

## Design
### Project Structure
The project was organized into numerous modular C source and header files, each responsible for a specific system function on the STM32 microcontroller. This modular design imrpoves readibility and debugging.

#### TIM.c

This file configures Timer 4, enabling its clock and setting up trigger output (TRGO). The ARR and CCR1 values are adjusted to generate a 440 Hz tone for the buzzer when the proximity threshold is met. It also configures TIM2 Channel 1 for debugging purposes.

#### UART.c

Configures USART2 for serial communication using alternate function mode on pins PA2 (TX) and PA3 (RX). This enables data transmission between the STM32 board and Tera Term for real-time monitoring and debugging.

#### ADC.c

Sets up the Analog-to-Digital Converter (ADC) on pin PA1. The ADC converts continuous analog voltage signals from the infrared sensor into discrete digital values used for proximity detection.

#### DAC.c

Configures DAC Channel 2 on PA5, triggered by Timer 4 TRGO events. The DAC converts 12-bit digital values (0–4095) into corresponding analog voltage outputs, supporting signal generation and control within the system.

#### LED.c

Handles configuration and control of the onboard green LED connected to PA5. It includes functions to turn the LED on, off, and toggle its state. This file relies on definitions from LED.h.

#### SysClock.c

Configures the system clock by enabling the PLL and setting the system frequency to 80 MHz. This ensures consistent timing across all peripherals and system operations. It uses definitions from SysClock.h.

#### SysTimer.c

Implements a system delay mechanism using a reload-based timer. It provides timing utilities for delays and scheduling and depends on SysTimer.h, LED.h, and main.h.

#### main.c

The main control file that integrates all system components and executes the four project tasks. It includes and coordinates all peripheral headers such as:
stm32fxxx.h, SysClock.h, LED.h, UART.h, SysTimer.h, DAC.h, ADC.h, main.h, along with standard libraries (string.h, stdio.h).

This file also contains the ADC interrupt handler and the TIM4 interrupt handler, which manage real-time sensor reading and timer-triggered events.

### Code Description

#### System Clock Configuration

The System_Clock_Init() function configures the STM32 system clock to operate at 80 MHz using the Phase-Locked Loop (PLL). This clock serves as the timing source for all peripherals used throughout the project.

#### UART Communication

The UART2_Init() function initializes USART2 for serial communication with Tera Term. It calls USART2_GPIO_Init() to configure pins PA2 (TX) and PA3 (RX) for Alternate Function 7 and USART_Init() to configure the USART peripheral. This module allows sensor voltage readings and system status information to be displayed on a terminal window.

#### Timer Configuration

The TIM4_Init() function configures Timer 4 Channel 1 as the primary timing source for the project. Timer 4 generates TRGO (Trigger Output) events that simultaneously trigger both ADC and DAC operations. The timer's ARR and CCR1 values are selected to generate a 440 Hz output frequency for the buzzer. Additionally, TIM2 Channel 1 is configured for debugging and signal verification.

<p align="center">
  <img width="669" height="104" alt="image" src="https://github.com/user-attachments/assets/fd24b973-d337-43fa-9f91-09c8b0e88681" />
</p>

#### LED Control

The configure_LED_pin() function configures the onboard green LED connected to PA5. Additional functions including turn_on_LED(), turn_off_LED(), and toggle_LED() provide simple control of the LED state. The LED serves as a visual indicator when the proximity threshold has been exceeded.

#### SysTick Timer

The SysTick_Init() function configures the SysTick timer to generate a 1 ms time base derived from the 80 MHz system clock. This timing source is used for software delays and periodic tasks throughout the application.

#### ADC Configuration

The ADC module is responsible for reading the voltage produced by the infrared sensor.

- ADC_Wakeup() exits the ADC from deep-power-down mode.
- ADC_Common_Configuration() configures common ADC settings.
- ADC_Pin_Init() configures PA1 as ADC input channel ADC12_IN6.
- ADC_Init() enables the ADC, selects 12-bit resolution, and configures Timer 4 TRGO as the conversion trigger source.

The ADC converts the analog sensor voltage into a digital value ranging from 0 to 4095, allowing the software to determine whether an object is within the specified detection range.

#### DAC Configuration

The DAC module generates the signal used to drive the buzzer.

- DAC_Pin_Configuration() configures the DAC output pin for analog operation. 
- DAC_Configuration() enables and calibrates DAC Channel 2.

Timer 4 TRGO events trigger DAC updates, allowing a waveform to be generated at the desired frequency. The DAC operates in 12-bit mode with output values ranging from 0 to 4095.

#### Main Program 

The main() function initializes all system peripherals, including the system clock, UART, ADC, DAC, timers, SysTick, and LED modules.

After initialization, the program continuously monitors the infrared sensor readings. When the measured ADC value exceeds the predefined threshold, the system:

- Activates the 440 Hz buzzer
- Turns on the green LED
- Reports sensor voltage measurements through Tera Term

If the threshold is not met, the buzzer is disabled and the LED is turned off.

Sensor voltages are calculated from the ADC readings using the 12-bit conversion relationship:

<p align="center">
  <img width="652" height="58" alt="image" src="https://github.com/user-attachments/assets/e3acb63d-998d-42e2-96f1-1718cdc3ea0e" />
</p>

Voltage measurements are transmitted to Tera Term approximately every 500 ms for real-time monitoring.

#### Timer Interrupt Handler

The TIM4_IRQHandler() function executes whenever Timer 4 generates an interrupt. This handler updates the DAC output value and controls whether the buzzer waveform is generated. If an object is detected, the DAC outputs the waveform required for the 440 Hz tone; otherwise, the DAC output remains at 0 V.

#### ADC Interrupt Handler

The ADC1_2_IRQHandler() function processes ADC conversion results. Each time a conversion completes, the handler compares the measured value against a predefined threshold. If the sensor voltage indicates that an object is within range, a flag is set to enable the buzzer and LED. Otherwise, the flag is cleared, disabling both outputs.

This interrupt-driven approach allows the system to respond quickly to changes in object proximity while minimizing processor overhead.

## Experiment, Results, and Conclusions

### Parts List

- 1x stm32l476_NUCLEO_L476RG microcontroller kit
- 1x Bread board
- 1x 10k Ohm Resistor
- 1x 1k Ohm Resistor
- 1x 10M Ohm Resistor
- 1x Photodiode
- 1x LED
- Male to Male Wires
- 1x Buzzer

### Circuit Implementation

#### Figure 1: Breadboard Circuit with Code
<table>
  <tr>
    <td align="center">
      <img width="948" height="605" alt="image" src="https://github.com/user-attachments/assets/51645de5-9b25-4af7-bc40-f2275ac46e98" /><br>
      Before Threshold activated
    </td>
    <td align="center">
      <img width="653" height="637" alt="image" src="https://github.com/user-attachments/assets/b21160c3-6f8b-4a05-9b72-48cc7d249f66" /><br>
      After 3 V Threshold activated
    </td>
  </tr>
</table>

### Testing

There are a few ways to see if the code is implemented on the board correctly. The first is if LED D5 is turned off and the buzzer is quiet after the code is initially loaded onto the board. After the reset (black) button is pushed the LED will turn green and buzzer will generate a tone with 440 Hz whenever an 
object approaches the threshold of the sensor, which is set to 3 V. The LED and buzzer should not work if there is nothing close to the sensor. The final way to see if the code is implemented correctly is if Tera Term displays the correct measured volts by ADC whenever an object approaches the sensor. 

<p align="center">
  <img width="594" height="470" alt="image" src="https://github.com/user-attachments/assets/7496427c-d7b4-41aa-a111-fb66d1dc1342" /> <br>
  <b>Fig 2. Tera Term Terminal with measured volts</b>
</p>

### Demo Video

[![Watch the demo](https://img.youtube.com/vi/6O10AegdtcA/maxresdefault.jpg)](https://www.youtube.com/watch?v=6O10AegdtcA)

### Results

When the code is initially implemented on the board LED D5 is off and the buzzer is quiet, as seen in Figure 1. After the reset button is pushed LED D5 is still off and the buzzer is still quiet, as seen in Figure 1. Next, whenever I moved my hand towards the sensor, and it reaches the threshold (3 V) the green LED lights up and the buzzer starts generating a tone of 440 Hz, as seen in Figure 1. Finally, the measured volts changed based on the distance between my hand and the sensor, which is displayed in Tera Tern, as seen in Figure 2. 

### Conclusions

ADC and DAC signals are triggered by a timer. This trigger allows ADC conversion and DAC tone generation in the code. By setting a threshold to the sensor it is also possible to control whenever DAC voltage is present in the circuit. 

## References

[1] “STM32L476RG_NUCLEO_Pins” ENGIN 346 on Canvas of UMass Boston, 2024 Fall, umassboston.instructure.com/courses/3405/pages/datasheets  
[2] “STM32L47xxx_Reference_Manual” ENGIN 346 on Canvas of UMass Boston, 2024 Fall, umassboston.instructure.com/courses/3405/pages/datasheets  
