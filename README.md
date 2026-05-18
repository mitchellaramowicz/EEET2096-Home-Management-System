# EEET2096 - Home Management System

A project for STM32F439 that implements automated temperature control and light/fan logic with UART communication.

### Authors
- Justin Nguyen
- Mitchell Aramowicz

### Features
- **Temperature Control**: Automated heater and cooler management based on ADC temperature sensor readings.
- **Light & Fan Logic**: Control logic for lighting and fans based on sensor inputs and manual overrides.
- **UART Interface**: Remote monitoring and control via UART packets (0.25 Hz transmission rate).
- **Manual Overrides**: Button debouncing and manual lockout mechanisms for user interaction.