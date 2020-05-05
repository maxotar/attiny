#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "pin.h"
#include "button.h"
#include "random.h"

#define NUM_PINS 4
#define NUM_SUCCESS 1
#define NUM_FAILURE 5
#define NUM_PATTERN_INITIAL 1
#define TICKS_SUCCESS 64
#define TICKS_FAILURE 8
#define TICKS_PATTERN_INITIAL 50
#define TICKS_TO_SPEED_UP 0
#define TIMEOUT 5

const Pin pins[NUM_PINS] = {
	{.bit = 4, .vport = &VPORTA, .port = &PORTA},
	{.bit = 5, .vport = &VPORTA, .port = &PORTA},
	{.bit = 6, .vport = &VPORTA, .port = &PORTA},
	{.bit = 7, .vport = &VPORTA, .port = &PORTA},
};

uint8_t seed;
uint8_t pattern;
uint8_t count;
uint8_t count_to_win;
uint8_t ticks;
uint8_t ticks_to_show_pattern;
uint8_t seconds_elapsed;
uint8_t histories[NUM_PINS];

typedef void* StateFunction(void);
void light_on(uint8_t i);
void light_off(uint8_t i);
void lights_on(void);
void lights_off(void);
void cfg_hibernate(void);
void cfg_initialize(void);
void* state_hibernating(void);
void* state_entering_pattern(void);
void* state_indicating_pattern(void);
void* state_indicating_success(void);
void* state_indicating_failure(void);

int main(void)
{
	sei();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	cfg_hibernate();
	StateFunction *state = state_hibernating;
	
	while (1)
	{
		sleep_cpu();
		state = state();
	}
}

ISR(RTC_PIT_vect)
{
	RTC.PITINTFLAGS = RTC_PI_bm;
}

ISR(PORTA_PORT_vect) {}
ISR(PORTB_PORT_vect) {}
ISR(PORTC_PORT_vect) {}

void light_on(uint8_t i)
{
	cfg_output(pins[i]);
	drive_low(pins[i]);
}

void light_off(uint8_t i)
{
	cfg_input(pins[i]);
}

void lights_on(void)
{
	for (uint8_t i = 0; i < NUM_PINS; i++)
	{
		light_on(i);
	}
}

void lights_off(void)
{
	for (uint8_t i = 0; i < NUM_PINS; i++)
	{
		light_off(i);
	}
}

void cfg_hibernate(void)
{
	RTC.PITCTRLA = 0;
	RTC.PITINTCTRL = 0;
	while(RTC.PITSTATUS);

	for (uint8_t i = 0; i < 8; i++)
	{
		*(&PORTA.PIN0CTRL + i) = PORT_PULLUPEN_bm;
		*(&PORTB.PIN0CTRL + i) = PORT_PULLUPEN_bm;
		*(&PORTC.PIN0CTRL + i) = PORT_PULLUPEN_bm;
	}

	for (uint8_t i = 0; i < NUM_PINS; i++)
	{
		cfg_rising(pins[i]);
	}
}

void cfg_initialize(void)
{
	RTC.PITCTRLA = RTC_PERIOD_CYC256_gc | RTC_PITEN_bm;
	RTC.PITINTCTRL = RTC_PI_bm;
	while(RTC.PITSTATUS);
	seed += 255 / (NUM_PINS);
	set_seed(seed);
	pattern = seed / (255 / NUM_PINS + 1);
	ticks = 0;
	ticks_to_show_pattern = TICKS_PATTERN_INITIAL;
	count = 0;
	count_to_win = NUM_PATTERN_INITIAL;
}

void* state_hibernating(void)
{
	for (uint8_t i = 0; i < NUM_PINS; i++)
	{
		intflag_clear(pins[i]);
		cfg_intdisable(pins[i]);
	}
	
	cfg_initialize();
	return state_indicating_pattern;
}

void* state_indicating_failure(void)
{
	ticks++;
	if (ticks >= TICKS_FAILURE)
	{
		ticks = 0;
		count++;
		lights_off();
		if (count & 1)
		{
			light_on(pattern);
		}
		if ((count >> 1) >= NUM_FAILURE) {
			cfg_hibernate();
			return state_hibernating;
		}
	}
	return state_indicating_failure;
}

void* state_indicating_success(void)
{
	ticks++;
	if (ticks >= TICKS_SUCCESS)
	{
		ticks = 0;
		count++;
		lights_off();
		if (count & 1)
		{
			lights_on();
		}
		if ((count >> 1) >= NUM_SUCCESS)
		{
			set_seed(seed);
			pattern = seed / (255 / NUM_PINS + 1);
			count = 0;
			count_to_win++;
			ticks_to_show_pattern -= TICKS_TO_SPEED_UP;
			lights_off();
			return state_indicating_pattern;
		}
	}
	return state_indicating_success;
}

void* state_indicating_pattern(void)
{
	ticks++;
	if (ticks >= ticks_to_show_pattern)
	{
		ticks = 0;
		count++;
		lights_off();
		if (count & 1)
		{
			light_on(pattern);
			pattern = random_0_255() / (255 / NUM_PINS + 1);
		}
		if((count >> 1) >= count_to_win)
		{
			count = 0;
			seconds_elapsed = 0;
			set_seed(seed);
			pattern = seed / (255 / NUM_PINS + 1);
			lights_off();
			for (uint8_t i = 0; i < NUM_PINS; i++)
			{
				histories[i] = 0;
			}
			return state_entering_pattern;
		}
	}
	return state_indicating_pattern;
}

void* state_entering_pattern(void)
{
	for (uint8_t i = 0; i < NUM_PINS; i++)
	{
		button_update(&histories[i], pins[i]);
	}
	
	ticks++;
	if (ticks == 255)
	{
		seconds_elapsed++;
		if (seconds_elapsed >= TIMEOUT)
		{
			cfg_hibernate();
			return state_hibernating;
		}
	}
	
	for (uint8_t i = 0; i < NUM_PINS; i++)
	{
		if (button_was_released(&histories[i]))
		{
			if (i == pattern)
			{
				seconds_elapsed = 0;
				count++;
				if (count >= count_to_win)
				{
					ticks = 0;
					count = 0;
					lights_off();
					return state_indicating_success;
				}
				pattern = random_0_255() / (255 / NUM_PINS + 1);
				break;
			}
			else
			{
				ticks = 0;
				count = 0;
				lights_off();
				return state_indicating_failure;
			}
		}
	}
	
	return state_entering_pattern;
}
