/*
* HPCtl.c
*
* Created: 05.10.2023 19:40:37
* Author : falkb
*/

#ifndef F_CPU
# define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "twislave.h"
#include "relay.h"

#define PWM_TICK_FREQUENCY_HZ 1
#define PWM_PERIOD_SECONDS 10
// The number of heater levels, defined by the PWM cycle.
#define NUM_HEATER_LEVELS (PWM_TICK_FREQUENCY_HZ * PWM_PERIOD_SECONDS * 3 + 1)

// A tick-counter driven by a timer.
// This counts seconds modulo some number.
volatile uint8_t timer_ticks;

// Initializes outputs.
static void io_init() {
	// No pull ups necessary because of external pull ups.
	// We use pull ups anyway to make debugging without external pull ups easier.
	
	// Outputs:
	// PD0..PD4:			General purpose SSRs
	// PD5, PB6, PB7:		Heater SSRs
	// PD6, PD7, PB0..PB5:	Relais
	
	// Inputs:
	// PC0..PC3:			General purpose Inputs
	// PC4, PC5:			I2C
	
	// Vent pins high = vent off
	PORTB = 0b00111111;
	DDRB = 0xFF;
	
	// PC0 to PC5 as inputs with pull ups enabled
	PORTC = 0xFF;
	DDRC = 0x00;
	
	// Vent pins high = vent off
	PORTD = 0b11000000;
	DDRD = 0xFF;
	
}

static void init_timer() {
	// See http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
	// Or https://www.arduinoslovakia.eu/application/timer-calculator

	// ! Make sure this fits with PWM_TICK_FREQUENCY_HZ !

	TCCR1A = 0; // set entire TCCR1A register to 0
	TCCR1B = 0; // same for TCCR1B
	TCNT1 = 0; // initialize counter value to 0
	// set compare match register for 1 Hz increments
	OCR1A = 31249; // = 8000000 / (256 * 1) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12, CS11 and CS10 bits for 256 prescaler
	TCCR1B |= (1 << CS12);
	// enable timer compare interrupt
	TIMSK |= (1 << OCIE1A);
}

// Reads input pins PC0..PC3
static void readout() {
	// Read PC0..PC3
	uint8_t values = PINC & 0x0F;
	
	// Compute shifted complement
	uint8_t complement = ~values;
	complement = complement << 4;

	// Write to I2C buffer
	i2cdata[5] = complement | values;
}

static void compute_checksum(){
	cli();
	
	uint8_t sum = 0;
	sum += i2cdata[0];
	sum += i2cdata[1];
	sum += i2cdata[2];
	sum += i2cdata[3];
	i2cdata[4] = ~sum;
	
	sei();
}

static void set_heater(uint8_t level, uint8_t pwm_counter){
	if (level > 20) {
		PORTB |= (1<<PORTB6);
		PORTB |= (1<<PORTB7);
		uint8_t remainder = level - 20;
		if (remainder > pwm_counter) {
			PORTD |= (1<<PORTD5);
			} else {
			PORTD &= ~(1<<PORTD5);
		}
	}
	else {
		if (level > 10) {
			PORTB |= (1<<PORTB6);
			PORTD &= ~(1<<PORTD5);
			uint8_t remainder = level - 10;
			if (remainder > pwm_counter) {
				PORTB |= (1<<PORTB7);
				} else {
				PORTB &= ~(1<<PORTB7);
			}
		}
		else {
			PORTD &= ~(1<<PORTD5);
			PORTB &= ~(1<<PORTB7);
			if (level > pwm_counter) {
				PORTB |= (1<<PORTB6);
				} else {
				PORTB &= ~(1<<PORTB6);
			}
		}
	}

	// Expose ssr state to i2c.
	cli();
	i2cdata[3] &= 0b00011111;
	if(PORTB & (1<<PORTB6))
	i2cdata[3] |= (1<<5);
	if(PORTB & (1<<PORTB7))
	i2cdata[3] |= (1<<6);
	if(PORTD & (1<<PORTD5))
	i2cdata[3] |= (1<<7);
	sei();
}

static void set_ssrs(uint8_t value){
	// Mask out unaffected bits.
	uint8_t mask_reg_d = 0b00011111;
	value &= mask_reg_d;
	
	PORTD = (PORTD & ~mask_reg_d) | value;
}

ISR(TIMER1_COMPA_vect) // every 1s
{
	timer_ticks = (timer_ticks + 1) % (PWM_PERIOD_SECONDS * PWM_TICK_FREQUENCY_HZ);
}


int main(void)
{
	io_init();
	// Enable watchdog to restart if we didn't reset it for 2 seconds.
	wdt_enable(WDTO_2S);
	// Enable I2C.
	init_twi_slave(I2C_SLAVE_ADDRESS);
	// Read input values
	readout();
	// Init timer
	init_timer();
	
	// Enable Interrupts.
	sei();
	
	while (1)
	{
		// Reset watchdog timer.
		wdt_reset();
		
		readout();
		
		compute_checksum();
		
		set_heater(i2cdata[0], timer_ticks);
		
		set_air_in(i2cdata[1]);
		set_air_out(i2cdata[2]);
		
		set_ssrs(i2cdata[3]);
	}
}

