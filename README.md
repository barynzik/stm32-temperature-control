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

## Desktop GUI

A dedicated desktop graphical user interface (GUI) application was developed
to allow setting the temperature reference and monitoring the system signals
(measured temperature, reference value and control output).

The GUI is implemented in Python using Tkinter and supports both a demo mode
(without hardware) and serial communication mode.

The GUI source code and screenshots are located in the `pc_gui/` directory.

## Additional user output (LED)

The system implements an additional user output device in the form of
LED indicators on the STM32 NUCLEO board. The LEDs are used to visualize
the current system status.

- Normal operation: temperature within the safe range
- Alarm state: temperature outside the allowed limits

The LED logic is implemented in a dedicated firmware module and updated

## Additional control output (Fan)

An additional control output device was implemented in the form of a cooling fan.
The fan is automatically enabled when the measured temperature exceeds the
reference temperature by a configurable threshold.

The fan control logic is implemented as a separate module (`fan.c`, `fan.h`)
and integrated into the main control loop.

## Additional user input device

An additional user input device was implemented using an analog potentiometer
connected to the ADC input of the STM32 microcontroller.

The potentiometer allows the user to adjust the temperature set-point locally,
without using the serial interface. The ADC value is periodically sampled and
converted into a temperature reference value within the allowed control range.

This satisfies the requirement of an additional user input device.

## Serial communication with checksum

The UART communication protocol includes a checksum mechanism to ensure data
integrity. Each message contains a checksum field calculated using an XOR-based
algorithm. Received messages are validated before being processed, and invalid
frames are discarded.

This fulfills the requirement of using a checksum in serial communication.
