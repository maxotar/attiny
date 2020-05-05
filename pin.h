#ifndef PIN_H_
#define PIN_H_

#define _GET_BIT(port, bit) ((port >> bit) & 1)
#define _SET_BIT(port, bit) (port |= _BV(bit))
#define _CLR_BIT(port, bit) (port &= ~_BV(bit))
#define _TGL_BIT(port, bit) (port ^= _BV(bit))

#define GET_BIT(x) _GET_BIT(x)
#define SET_BIT(x) _SET_BIT(x)
#define CLR_BIT(x) _CLR_BIT(x)
#define TGL_BIT(x) _TGL_BIT(x)

typedef struct Pin
{
	VPORT_t* vport;
	PORT_t* port;
	uint8_t bit;
} Pin;

void drive_high(Pin pin)
{
	pin.vport->OUT |= _BV(pin.bit);
}

void drive_low(Pin pin)
{
	pin.vport->OUT &= ~_BV(pin.bit);
}

void drive_toggle(Pin pin)
{
	pin.vport->OUT ^= _BV(pin.bit);
}

void cfg_output(Pin pin)
{
	pin.vport->DIR |= _BV(pin.bit);
}

void cfg_input(Pin pin)
{
	pin.vport->DIR &= ~_BV(pin.bit);
}

void cfg_intdisable(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) = (*(&pin.port->PIN0CTRL + pin.bit) & ~PORT_ISC_gm);
}

void cfg_bothedges(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) = (*(&pin.port->PIN0CTRL + pin.bit) & ~PORT_ISC_gm) | 1;
}

void cfg_rising(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) = (*(&pin.port->PIN0CTRL + pin.bit) & ~PORT_ISC_gm) | 2;
}

void cfg_falling(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) = (*(&pin.port->PIN0CTRL + pin.bit) & ~PORT_ISC_gm) | 3;
}

void cfg_input_disable(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) = (*(&pin.port->PIN0CTRL + pin.bit) & ~PORT_ISC_gm) | 4;
}

void cfg_level(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) = (*(&pin.port->PIN0CTRL + pin.bit) & ~PORT_ISC_gm) | 5;
}

void cfg_pullup_enable(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) |= PORT_PULLUPEN_bm;
}

void cfg_pullup_disable(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) &= ~PORT_PULLUPEN_bm;
}

void cfg_logic_invert(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) |= PORT_INVEN_bm;
}

void cfg_logic_normal(Pin pin)
{
	*(&pin.port->PIN0CTRL + pin.bit) &= ~PORT_INVEN_bm;
}

uint8_t is_high(Pin pin)
{
	return (pin.vport->IN >> pin.bit) & 1;
}

uint8_t is_low(Pin pin)
{
	return (pin.vport->IN & _BV(pin.bit)) == 0;
}

uint8_t intflag_is_high(Pin pin)
{
	return (pin.vport->INTFLAGS >> pin.bit) & 1;
}

uint8_t intflag_is_low(Pin pin)
{
	return (pin.vport->INTFLAGS & _BV(pin.bit)) == 0;
}

void intflag_clear(Pin pin)
{
	pin.vport->INTFLAGS = _BV(pin.bit);
}

#endif /* PIN_H_ */