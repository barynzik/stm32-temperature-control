# STM32 Temperature Control System

Microcontroller-based temperature control system using an STM32 microcontroller, NTC thermistor,
PWM-controlled heating element and a PI controller.

## Features
- Closed-loop temperature control
- NTC temperature measurement using ADC
- PWM heater control via MOSFET
- UART communication for set-point and monitoring
- Modular firmware structure

## Hardware
- STM32 NUCLEO development board
- NTC 10k temperature sensor
- Resistive heating element
- N-channel MOSFET
- External DC power supply

## Software
- STM32CubeIDE
- C language
- Git version control

## Source code organization

The firmware source code is organized into separate modules with clear
separation of functionality and configuration. The project follows the
standard STM32CubeIDE structure (`Core`, `Drivers`) and uses Doxygen-style
comments for code documentation.

## Version control

The project is maintained using Git and hosted on GitHub. Version control is
used to track changes in firmware source code, simulation models and
documentation.

## Simulation and verification

The steady-state accuracy requirement (1% of control range) was verified using
a Simulink model of the thermal process and a PI controller. The simulation
includes a staircase temperature reference and demonstrates negligible
steady-state error for all operating points.

Simulation model and results are provided in the `sim/` directory.
