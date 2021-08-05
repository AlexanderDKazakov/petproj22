/*
  tools.h - Useful tools for the project {PETPROJ22}
  Part of PETPROJ22
  
  Copyright (c) 2021 Aleksandr D. Kazakov | https://github.com/AlexanderDKazakov/
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

*/

#include <Arduino.h>
#include <ModbusMaster.h>

#ifndef TIME_LAG
#define TIME_LAG   wait_for_ms(100)
#endif

#ifndef TIME_LAG2X
#define TIME_LAG2X wait_for_ms(200)
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


static void turnOffPWM(uint8_t timer)
{
	switch (timer)
	{
		#if defined(TCCR1A) && defined(COM1A1)
		case TIMER1A:   cbi(TCCR1A, COM1A1);    break;
		#endif
		#if defined(TCCR1A) && defined(COM1B1)
		case TIMER1B:   cbi(TCCR1A, COM1B1);    break;
		#endif
		#if defined(TCCR1A) && defined(COM1C1)
		case TIMER1C:   cbi(TCCR1A, COM1C1);    break;
		#endif
		
		#if defined(TCCR2) && defined(COM21)
		case  TIMER2:   cbi(TCCR2, COM21);      break;
		#endif
		
		#if defined(TCCR0A) && defined(COM0A1)
		case  TIMER0A:  cbi(TCCR0A, COM0A1);    break;
		#endif
		
		#if defined(TCCR0A) && defined(COM0B1)
		case  TIMER0B:  cbi(TCCR0A, COM0B1);    break;
		#endif
		#if defined(TCCR2A) && defined(COM2A1)
		case  TIMER2A:  cbi(TCCR2A, COM2A1);    break;
		#endif
		#if defined(TCCR2A) && defined(COM2B1)
		case  TIMER2B:  cbi(TCCR2A, COM2B1);    break;
		#endif
		
		#if defined(TCCR3A) && defined(COM3A1)
		case  TIMER3A:  cbi(TCCR3A, COM3A1);    break;
		#endif
		#if defined(TCCR3A) && defined(COM3B1)
		case  TIMER3B:  cbi(TCCR3A, COM3B1);    break;
		#endif
		#if defined(TCCR3A) && defined(COM3C1)
		case  TIMER3C:  cbi(TCCR3A, COM3C1);    break;
		#endif

		#if defined(TCCR4A) && defined(COM4A1)
		case  TIMER4A:  cbi(TCCR4A, COM4A1);    break;
		#endif					
		#if defined(TCCR4A) && defined(COM4B1)
		case  TIMER4B:  cbi(TCCR4A, COM4B1);    break;
		#endif
		#if defined(TCCR4A) && defined(COM4C1)
		case  TIMER4C:  cbi(TCCR4A, COM4C1);    break;
		#endif			
		#if defined(TCCR4C) && defined(COM4D1)
		case TIMER4D:	cbi(TCCR4C, COM4D1);	break;
		#endif			
			
		#if defined(TCCR5A)
		case  TIMER5A:  cbi(TCCR5A, COM5A1);    break;
		case  TIMER5B:  cbi(TCCR5A, COM5B1);    break;
		case  TIMER5C:  cbi(TCCR5A, COM5C1);    break;
		#endif
	}
}

void DigitalWrite(uint8_t pin, uint8_t val)
{
	uint8_t timer = digitalPinToTimer(pin);
	uint8_t bit   = digitalPinToBitMask(pin);
	uint8_t port  = digitalPinToPort(pin);
	volatile uint8_t *out;

	if (port == 0) return;

	// If the pin that support PWM output, we need to turn it off before doing a digital write.
	if (timer != 0) turnOffPWM(timer);
     
	out = portOutputRegister(port);
	uint8_t oldSREG = _SFR_IO8(0x3F);            // save SREG state
	__asm__ __volatile__ ("cli" ::: "memory");

	if   (val == 0x0) *out &= ~bit;
    else              *out |= bit;
	_SFR_IO8(0x3F) = oldSREG;                    // restore SREG state
}

// Error in several percent is not important.
void wait_for_ms(unsigned long ms)
{
	uint32_t start = micros();

	while (ms > 0) {
		yield();
		while ( ms > 0 && (micros() - start) >= 1000) {
			ms--;
			start += 1000;
		}
	}
}

void PinMode(uint8_t pin, uint8_t mode)
{
	uint8_t bit  = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	volatile uint8_t *reg, *out;

	if (port == 0) return;

	reg = portModeRegister(port);
	out = portOutputRegister(port);

	if (mode == INPUT) { 
		uint8_t oldSREG = _SFR_IO8(0x3F);          // save SREG state
        __asm__ __volatile__ ("cli" ::: "memory");
		*reg &= ~bit;
		*out &= ~bit;
		_SFR_IO8(0x3F) = oldSREG;                  // restore SREG state
	} else if (mode == INPUT_PULLUP) {
		uint8_t oldSREG = _SFR_IO8(0x3F);          // save SREG state
        __asm__ __volatile__ ("cli" ::: "memory");
		*reg &= ~bit;
		*out |= bit;
		_SFR_IO8(0x3F) = oldSREG;                  // restore SREG state
	} else {
		uint8_t oldSREG = _SFR_IO8(0x3F);          // save SREG state
        __asm__ __volatile__ ("cli" ::: "memory");
		*reg |= bit;
		_SFR_IO8(0x3F) = oldSREG;                  // restore SREG state
	}
}
