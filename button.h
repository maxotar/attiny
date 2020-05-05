#ifndef BUTTON_H_
#define BUTTON_H_

#include "pin.h"

#define BUTTON_MASK 0b11000011

void button_update(uint8_t* history, Pin pin)
{
	*history = (*history << 1) | is_low(pin);
}

uint8_t button_was_pressed(uint8_t* history)
{
	if ((*history & BUTTON_MASK) == 0b00000011)
	{
		*history = 0b11111111;
		return 1;
	}
	return 0;
}

uint8_t button_was_released(uint8_t* history)
{
	if ((*history & BUTTON_MASK) == 0b11000000)
	{
		*history = 0b00000000;
		return 1;
	}
	return 0;
}

uint8_t button_is_down(uint8_t history)
{
	return history == 0b11111111;
}

uint8_t button_is_up(uint8_t history)
{
	return history == 0b00000000;
}

#endif /* BUTTON_H_ */