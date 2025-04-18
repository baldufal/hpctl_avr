# HPCTL AVR
Software for Atmega48/88/168/328 controller of HPCTL heating and ventilation controller project.

We use it with Microchip Studio and Atmega168.

# Task
This mc is controlled by a [ESP32](https://github.com/baldufal/hpctl_arduino/tree/main) via I2C bus.
It drives 8 solid state relais (SSRs) and 8 mechanical relais and reports the state of 4 universal binary inputs.
3 of the SSRs are connected to a heater. They are driven with very slow PWM with a period of 10s.
The mechanical relais control one ventilator.
Another ventilator is controlled via PWM on one of the SSR outputs (SSR3/PD3).

# I2C Interface
The I2C interface exposes 6 8bit registers:
Register | Semantics | Allowed values (writing)
---|---|---
0 | Heater level | 0-30
1 | Vent IN | 0-255
2 | Vent OUT | 0-6
3 | SSRs | 0b000x0xxx
4 | Checksum | -
5 | Inputs | -

- The three most significant bits of register 3 are determined by heater level. They cannot be set manually, but their current values can be read from this register.
- Similarly for 4th bit (0b00001000) is controlled by the Vent OUT level (1kHz PWM).
- The checksum is the inverse of the 8bit sum of registers 0 to 3.
- The lower 4 bits of register 5 contain the actual input states.
- The upper 4 bits of register 5 contain the bitwise inverse of the input states.

# Board
We use the controller on a custom PCB. It additionally provides the corresponding ESP32 with 5V supply voltage and a debounced terminal for 4 buttons which are connected directly to the ESP32.

![schematic](./Board/HPCTL&#32;Schematic.png)

![board](./Board/HPCTL&#32;Board.png)
The layout file uses a wrong footprint for the mC.
