//
// Created by falk on 06/10/23.
//
// We have eight relays. The first four deal with air intake, the last four with air out.
// All relays are in their default state if the corresponding pins are HIGH.
// Relays are connected to PORTB and PORTD.
//
// ==============================
// AIR INTAKE
// ==============================
//
// Relays 1 and 3 have power inputs for air intake.
// Relay 1:
// - Not connected (default)
// - 100V (active)
// Relay 3:
// - 190V (default)
// - 230V (active)
//
// Relay 2 sits above that to multiplex. It has relay 1 selected by default.
// Relay 3 demuxes the output of relay 2 to the two fan modes:
// - Intake air fan mode 1 (default)
// - Intake air fan mode 2 (active)
//
// We thus have six air intake modes, plus off:
// - off: all relays in default position
// - mode 1: 100V + fan mode 1
// - mode 2: 100V + fan mode 2
// - mode 3: 190V + fan mode 1
// - mode 4: 230V + fan mode 1
// - mode 5: 190V + fan mode 2
// - mode 6: 230V + fan mode 2
//
// Additionally, there is one connection between the fan-controlling MCU and the heater-controlling MCU, which is
// hardwired to relay 1, with a pullup.
// This connection is used as a safety feature: Only if the pin is LOW will the heating run.
// This means that, even if the "path" via relay 1 is unused, we still need to actively drive it LOW to indicate that
// the fan is running.
//
// ==============================
// AIR OUT
// ==============================
//
// Relays 5,6, and 8 have power inputs. All arrays are arranged like a tree again:
// Relay 5:
// - Not connected (default)
// - 80 V (active)
// Relay 7:
// - 120V (default)
// - 150V (active)
// Relay 8:
// - [output of relay 6] (default)
// - 230V (active)
// Relay 6 multiplexes between relays 5 and 7.
//
// We thus end up with 4 air-out modes, plus off.

#ifndef FAN_CONTROL_RELAY_H
#define FAN_CONTROL_RELAY_H

#include <avr/io.h>

// Our relays are HIGH by default, i.e., all of this is actually inverted.
#define RELAY_0 (1 << PORTB0)
#define RELAY_1 (1 << PORTD7) // PORT _D_
#define RELAY_2 (1 << PORTD6) // PORT _D_
#define RELAY_3 (1 << PORTB1)
#define RELAY_4 (1 << PORTB2)
#define RELAY_5 (1 << PORTB3)
#define RELAY_6 (1 << PORTB4)
#define RELAY_7 (1 << PORTB5)


#define NUM_AIR_IN_MODES 7
#define NUM_AIR_OUT_MODES 5

#define MASK_IN_B 0b00000011
#define MASK_IN_D 0b11000000
#define MASK_OUT_B 0b00111100


static void set_air_in(uint8_t level) {
	uint8_t reg_b = MASK_IN_B;
	uint8_t reg_d = MASK_IN_D;
	
	switch (level) {
		case 0:
		//return 0;
		break;
		case 1:
		//return RELAY_0;
		reg_b &= ~(RELAY_0);
		break;
		case 2:
		//return RELAY_0 | RELAY_3;
		reg_b &= ~(RELAY_0 | RELAY_3);
		break;
		case 3:
		//return RELAY_0 | RELAY_1;
		reg_b &= ~RELAY_0;
		reg_d &= ~RELAY_1;
		break;
		case 4:
		//return RELAY_0 | RELAY_2 | RELAY_1;
		reg_b &= ~RELAY_0;
		reg_d &= ~(RELAY_1 | RELAY_2);
		break;
		case 5:
		//return RELAY_0 | RELAY_1 | RELAY_3;
		reg_b &= ~(RELAY_0 | RELAY_3);
		reg_d &= ~RELAY_1;
		break;
		case 6:
		//return RELAY_0 | RELAY_1 | RELAY_2 | RELAY_3;
		reg_b &= ~(RELAY_0 | RELAY_3);
		reg_d &= ~(RELAY_1 | RELAY_2);
		break;
	}
	
	// Set registers without transient states and without affecting other bits
	reg_b = reg_b | (PORTB & ~MASK_IN_B);
	PORTB = reg_b;
	reg_d = reg_d | (PORTD & ~MASK_IN_D);
	PORTD = reg_d;
}


static void set_air_out(uint8_t level) {
	uint8_t reg_b = MASK_OUT_B;
	
	switch (level) {
		case 0:
		//return 0;
		break;
		case 1:
		//return RELAY_4;
		reg_b &= ~RELAY_4;
		break;
		case 2:
		//return RELAY_5;
		reg_b &= ~RELAY_5;
		break;
		case 3:
		//return RELAY_5 | RELAY_6;
		reg_b &= ~(RELAY_5 | RELAY_6);
		break;
		case 4:
		//return RELAY_7;
		reg_b &= ~RELAY_7;
		break;
	}
	
	// Set register without transient states and without affecting other bits
	reg_b = reg_b | (PORTB & ~MASK_OUT_B);
	PORTB = reg_b;
}


#endif //FAN_CONTROL_RELAY_H
